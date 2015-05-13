// Lab5_WINAPI_CLIENT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <glut.h> 
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#define PORT 80
#define SERVERADDR "127.0.0.1"
#pragma warning(disable:4996)

struct    cell          // ��������� ������
{
	float	lbx; //����� ������ ���� ������ (���������� �� �)
	float lby; //����� ������ ���� ������(���������� �� y)
	int hline = 0; //�����-��������� ������������� �����
	int  condition = 0; //��������� ������  (������� ���� ������; �� ������� ��� ������ ������� �����) 
	bool lock = false; //������������ ������ �� ������� ���� (�� ������� ��� ������ ��������)
};
cell cells[10][10];                // ������� ����� �������� ����

int cnt1, cnt2, cnt3, cnt4;			//���������� ��� �������� ����� ������� 
int nclients = 0;
bool checkMouse(int mx, int my);
void GetNumber(SOCKET my_sock);
char *Parser(char *request);
float mx = 0.0, my = 0.0;		//���������� ��� ��������� ����
float mx1 = 0.0, my1 = 0.0;		//���������� ��� ��������� ���� ������� ������
int count = 1;            //������� ����� �������
bool pp;
bool start = false;         // ����������, ����������� �������� ��������� �������� 
bool    keys[256];         // ������ ��� ����������� � �����������
int i, j;					//���������� ��� ������ ������
float down = false;             //������� �� ������
bool action = false;
bool zone = true;        //���������� ��� ����������� ����������� ����
int number;


//������� ��� ������ � ��������
DWORD WINAPI ToServer(LPVOID client_socket)
{
	char buff[1024];
	int bytes_recv;
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	GetNumber(my_sock);//�������� �� ������� ���� �����...
	//...� �������� ���� (�������� � ��������� ������)
	while (true)
	{
		//���� ����� ��������� ����������
		if (pp)
		{
			//������ ������� �� ����������
			char data[20];
			char lenght[10], MX[5], MY[5], NUMBER[5];
			itoa(mx, MX, 10);
			itoa(my, MY, 10);
			itoa(number, NUMBER, 10);
			strcpy(data, MX);
			strcat(data," ");
			strcat(data, MY);
			strcat(data, " ");
			strcat(data, NUMBER);
			itoa(sizeof data, lenght, 10);
			char result[1024];
			strcpy(result,"GET /send HTTP/1.1\r\n");
			strcat(result, "Host: ");
			strcat(result, SERVERADDR);
			strcat(result, "\r\n");
			strcat(result, "Accept: */*\r\n");
			strcat(result, "Connection: keep-alive\r\n");
			strcat(result, "Content-Length:");
			strcat(result, lenght);
			strcat(result, "\r\n");
			strcat(result, "Content:");
			strcat(result, data);
			strcat(result, "\r\n\r\n");
			send(my_sock,result,sizeof result, 0);
			//����� �������
			printf("Sending");
			bytes_recv = recv(my_sock,buff, sizeof buff,0);
			//printf(buff);
			bool accept = true;
			for (i = 0; i < sizeof buff; i++)
			{
				if (buff[i] == 'S' && buff[i + 1] == 'T' && buff[i + 2] == 'O')
				{
					accept = false;
					i = sizeof buff;
				}
			}
			if (accept==false)
				pp = false;

		}
		//���� ����� ������ �� ����� ������ ����
		if (number != count)
		{
			char data[1];
			char lenght[10], NUMBER[1];
			itoa(number, NUMBER, 10);
			strcpy(data, NUMBER);
			//������ ������� �� ��������� ������
			char result[1024];
			strcpy(result, "GET /recv HTTP/1.1\r\n");
			strcat(result, "Host: ");
			strcat(result, SERVERADDR);
			strcat(result, "\r\n");
			strcat(result, "Accept: */*\r\n");
			strcat(result, "Connection: keep-alive\r\n");
			strcat(result, "Content-Length:");
			strcat(result, lenght);
			strcat(result, "\r\n");
			strcat(result, "Content:");
			strcat(result, data);
			strcat(result, "\r\n\r\n");
			send(my_sock, result, sizeof result, 0);
			//����� �������
			bytes_recv = recv(my_sock, buff, sizeof buff, 0);
			//printf("\n%s\n",buff);
			bool accept = true;
			for (i = 0; i < sizeof buff; i++)
			{
				if (buff[i] == 'N' && buff[i+1]=='O')
				{
					accept = false;
					i = sizeof buff;
				}
			}
			if (accept==true)
			{
				char *data = Parser(buff);
				printf("\nI GOT %s\n", data);
				char newdata[15];
				strcpy(newdata, data);
				int y,z;
				bool accept=true;
				char s1[5];
				char s2[5];
				for (i = 0; i < sizeof newdata; i++)
				{
					if (newdata[i] == ' ')
					{
						if (accept)
						{
							y = i;
							accept = false;
						}
						else
						{
							z = i;
							i = sizeof newdata;
						}	
					}
					
				}
				printf("\nY %d Z %d\n",y,z);

				char *finalstr = new char(y);
				strncpy(finalstr, newdata,y);
				int mx1 = atoi(finalstr);
				printf("\nMX %d\n",mx1);
				char *finalstr1 = new char(z-y);
				strncpy(finalstr1, &newdata[y], z-y);
				int my1 = atoi(finalstr1);
				printf("\nMY %d\n", my1);

				if (checkMouse(mx1, my1))
				{
					if (count != 4)
						count++;
					else
						count = 1;
				}
			}

		}

	}
return 0;
}

//������ ��������
char *Parser(char *request)
{
	char msg[1024];
	strcpy(msg, request);
	char value[20];
	bool accept = true;
	for (int i = sizeof msg; i > 0; i--)
	{
		//printf("\ninside the loop\n");
		if (msg[i] == ':' && accept)
		{
			int j = 0;
			while (msg[i + 1] != '\r')
			{
				value[j] = msg[i + 1];
				i++;
				j++;
			}
			value[j] = '\0';
			char *finalstr = new char(j);
			strcpy(finalstr, value);
			printf("\nRESULT %s\n", finalstr);
			accept = false;
			i = sizeof msg;
			strcpy(msg, finalstr);
		}
	}
	return msg;
}


//������� ��������� ������ 
void GetNumber(SOCKET my_sock)
{
	char result[1024];
	char buff[1024];
	printf("Wait for number");
	strcpy(result, "GET /number HTTP/1.1\r\n");
	strcat(result, "Host:");
	strcat(result, SERVERADDR);
	strcat(result, "\r\n");
	strcat(result, "Accept: */*\r\n");
	strcat(result, "Connection: keep-alive\r\n");
	send(my_sock, result, sizeof result, 0);
	int bytes_recv = recv(my_sock, buff, sizeof buff, 0);
	//printf("\n%s", buff);
	char numb[1];
	bool accept = true;
	for (int i = sizeof buff; i > 0; i--)
	{
		if (buff[i] == ':' && accept)
		{
			numb[0] = buff[i + 1];
			number = atoi(numb);
			glutPostRedisplay();
			printf("\nMy number %d\n", number);
			accept = false;
		}
		//printf("\nChar number %s\n",numb);
	}
	
}




void SendDate(SOCKET my_sock)
{
	int nsize = 100;
	char buff[1024];
	char query[200];
	strcpy(buff, "\nGET /");
	strcat(buff, "send");
	strcat(buff, " HTTP/1.0\nHost: ");
	strcat(buff, SERVERADDR);
	strcat(buff, "\nUser-agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)");
	strcat(buff, "\nAccept: */*");
	strcat(buff, "\nContent: ");
	//strcat(query, buff);
	buff[strlen(query)] = '\0';
	send(my_sock, buff, sizeof(buff), 0);
}

void ReSizeGLScene()
{
	//glViewport(0, 0, width, height);  // ����� ������� ������� ���������
	glMatrixMode(GL_PROJECTION);   // ����� ������� ��������������
	glLoadIdentity();              // ����� ������� ��������������
	//glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f); // �������� �����. ���� 640x480 (0,0 � ���� ����)
	glMatrixMode(GL_MODELVIEW);    // ����� ������� ��������� ����
	glLoadIdentity();              // ����� ������� ��������� ����
}

// ��� ��������� ��� OpenGL �������� �����
int InitGL()
{
	glShadeModel(GL_SMOOTH);    // ��������� ������� �����������
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);   // ������ ���
	glClearDepth(1.0f);         // ��������� ������ �������
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // ����������� �����
	glEnable(GL_BLEND);         // ��������� ����������
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ��� ����������
	return 0;                // ������������� �������� �������
}

//������� ������������ ������
void drawcell(int i, int j)
{
	glBegin(GL_QUADS);
	glVertex2d(20 + (i * 60), 70 + (j * 60));
	glVertex2d(80 + (i * 60), 70 + (j * 60));
	glVertex2d(80 + (i * 60), 130 + (j * 60));
	glVertex2d(20 + (i * 60), 130 + (j * 60));
	glEnd();
}

//��������� ��������� �����
void drawhline(int i, int j, int condition)
{
	if (j < 10 && i < 10)
	{
		glLineWidth(3.0f);               // ������ ����� 3.0f  
		switch (condition)
		{
		case 1: glColor3f(1.0f, 0.0f, 0.0f); break;
		case 2: glColor3f(0.0f, 1.0f, 0.0f); break;
		case 3: glColor3f(0.0f, 0.0f, 1.0f); break;
		case 4: glColor3f(1.0f, 1.0f, 0.0f); break;
		default: break;
		}


		glBegin(GL_LINES);           // ������ ��������� ��������������� ������� ������
		glVertex2d(20 + (i * 60), 70 + (j * 60)); // ����� ������� �������������� �����
		glVertex2d(80 + (i * 60), 70 + (j * 60)); // ������ ������� �������������� �����
		glVertex2d(20 + (i * 60), 70 + ((j + 1) * 60)); // ������ �����
		glVertex2d(80 + (i * 60), 70 + ((j + 1) * 60)); // 
		glEnd();                     // ����� ��������� ��������������� ������� ������
	}
	if (i < 10 && j < 10)
	{
		glLineWidth(3.0f);               // ������ ����� 3.0f
		switch (condition)
		{
		case 1: glColor3f(1.0f, 0.0f, 0.0f); break;
		case 2: glColor3f(0.0f, 1.0f, 0.0f); break;
		case 3: glColor3f(0.0f, 0.0f, 1.0f); break;
		case 4: glColor3f(1.0f, 1.0f, 0.0f); break;
		default: break;
		}
		glBegin(GL_LINES);           // ������ ��������� ������������� ������� ������
		glVertex2d(20 + (i * 60), 70 + (j * 60));  // ������� ������� ������������ �����
		glVertex2d(20 + (i * 60), 130 + (j * 60)); // ������ ������� ������������ �����
		glVertex2d(20 + ((i + 1) * 60), 70 + (j * 60));  // ������� ������� ������������ �����
		glVertex2d(20 + ((i + 1) * 60), 130 + (j * 60)); // ������ ������� ������������ �����
		glEnd();                     // ����� ��������� ������������� ������� ������*/
	}
}

//������� ��������� �����
void drawline(int i, int j)
{
	if (j < 10 && i < 10)
	{
		glLineWidth(3.0f);               // ������ ����� 3.0f  
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);           // ������ ��������� ��������������� ������� ������
		glVertex2d(20 + (i * 60), 70 + (j * 60)); // ����� ������� �������������� �����
		glVertex2d(80 + (i * 60), 70 + (j * 60)); // ������ ������� �������������� �����
		glVertex2d(20 + (i * 60), 70 + ((j + 1) * 60)); // ������ �����
		glVertex2d(80 + (i * 60), 70 + ((j + 1) * 60)); // 
		glEnd();                     // ����� ��������� ��������������� ������� ������
	}
	if (i < 10 && j < 10)
	{
		glLineWidth(3.0f);               // ������ ����� 3.0f
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);           // ������ ��������� ������������� ������� ������
		glVertex2d(20 + (i * 60), 70 + (j * 60));  // ������� ������� ������������ �����
		glVertex2d(20 + (i * 60), 130 + (j * 60)); // ������ ������� ������������ �����
		glVertex2d(20 + ((i + 1) * 60), 70 + (j * 60));  // ������� ������� ������������ �����
		glVertex2d(20 + ((i + 1) * 60), 130 + (j * 60)); // ������ ������� ������������ �����
		glEnd();                     // ����� ��������� ������������� ������� ������*/
	}
}

//������� ������� ����� �����
 bool checkneighbors(int i, int j, int parity)
{
	bool accept = true;
	if (i > 0 && cells[i - 1][j].condition != 0)
	{
		if ((cells[i - 1][j].condition) == parity)
		{
			accept = false;
		}
	}
	if (cells[i + 1][ j].condition != 0 && i < 10)
	{
		if ((cells[i + 1][ j].condition) == parity)
		{
			accept = false;
		}
	}
	if (j > 0 && cells[i][ j - 1].condition != 0)
	{
		if ((cells[i][j - 1].condition) == parity)
		{
			accept = false;
		}
	}
	if (j < 10 && cells[i][ j + 1].condition != 0)
	{
		if ((cells[i][ j + 1].condition) == parity)
		{
			accept = false;
		}
	}
	//������������ ������
	if (i > 0 && j > 0 && cells[i - 1][ j - 1].condition != 0)
	{
		if ((cells[i - 1][ j - 1].condition) == parity)
		{
			accept = false;
		}
	}
	if (i < 10 && j < 10 && cells[i + 1][ j + 1].condition != 0)
	{
		if ((cells[i + 1][ j + 1].condition) == parity)
		{
			accept = false;
		}
	}
	if (i < 10 && j > 0 && cells[i + 1][ j - 1].condition != 0)
	{
		if ((cells[i + 1][ j - 1].condition) == parity)
		{
			accept = false;
		}
	}
	if (i > 0 && j < 10 && cells[i - 1][ j + 1].condition != 0)
	{
		if ((cells[i - 1][ j + 1].condition) == parity)
		{
			accept = false;
		}
	}
	if (accept == false)
		return true;
	else
		return false;
}

//������� ������������� (������ ������������ ����� ����)
 void fillneighbors(int i, int j)
{
	//�������� �������� ����� (�������� ������������)
	if (i > 0 && cells[i - 1][ j].condition != 0)
	{
		if (cells[i][ j].condition != cells[i - 1][ j].condition && i > 0)
			cells[i - 1][ j].condition = cells[i][ j].condition;
	}
	if (cells[i + 1][ j].condition != 0)
	{
		if (cells[i][ j].condition != cells[i + 1][ j].condition && i < 10)
			cells[i + 1][ j].condition = cells[i][j].condition;
	}
	if (j > 0 && cells[i][ j - 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i][ j - 1].condition && j > 0)
			cells[i][ j - 1].condition = cells[i][ j].condition;
	}
	if (cells[i][ j + 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i][ j + 1].condition && j > 0)
			cells[i][ j + 1].condition = cells[i][ j].condition;
	}
	//������������ ������
	if (i > 0 && j > 0 && cells[i - 1][ j - 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i - 1][ j - 1].condition && i > 0 && j > 0)
			cells[i - 1][ j - 1].condition = cells[i][ j].condition;
	}
	if (i < 10 && j < 10 && cells[i + 1][ j + 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i + 1][ j + 1].condition && i < 10 && j < 10)
			cells[i + 1][ j + 1].condition = cells[i][ j].condition;
	}
	if (i < 10 && j > 0 && cells[i + 1][ j - 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i + 1][ j - 1].condition && i < 10 && j > 0)
			cells[i + 1][ j - 1].condition = cells[i][ j].condition;
	}
	if (i > 0 && j < 10 && cells[i - 1][ j + 1].condition != 0)
	{
		if (cells[i][ j].condition != cells[i - 1][ j + 1].condition && i > 0 && j < 10)
			cells[i - 1][ j + 1].condition = cells[i][ j].condition;
	}
}

//������� �������� �������� ��������� � ����� ����
 bool checkMouse(int mx, int my)
{
	for (i = 0; i < 11; i++)   // ���� ����� �������
	{
		for (j = 0; j < 11; j++) // ���� ������ ����
		{
			if (i < 10 && j < 10)
			{
				cells[i][ j].lbx = 20 + (i * 60);
				cells[i][ j].lby = 50 + (j * 45);
			}
			//�������� �� ��������� ����� ������� � ���������� ������
			if (/*mx>20 && my>50 && mx<620 && my<500*/mx > cells[i][ j].lbx && mx < cells[i][ j].lbx + 60 && my > cells[i][ j].lby && my < cells[i][ j].lby + 45 && cells[i][ j].lock != true
				&& checkneighbors(i, j, count) == true)
			{

				switch (count)
				{
				case 1:
					cells[i][ j].condition = 1; //������ ��������� (����) ������
					fillneighbors(i, j); //����������� ��������� �������� ������; 
					break;
				case 2:
					cells[i][ j].condition = 2; //������ ��������� (����) ������
					fillneighbors(i, j); //����������� ��������� �������� ������
					break;
				case 3:
					cells[i][ j].condition = 3; //������ ��������� (����) ������
					fillneighbors(i, j); //����������� ��������� �������� ������
					break;
				case 4:
					cells[i][ j].condition = 4; //������ ��������� (����) ������
					fillneighbors(i, j); //����������� ��������� �������� ������
					break;
				default: break;
				}
				glutPostRedisplay();
				return true;
			}
		}
	}
	return false;

}

//�������, �������������� �������� � ����� ����
 void MousePressed(int button, int state, int ax, int ay)
{
	down = button == GLUT_LEFT_BUTTON && state == GLUT_LEFT; //���� ������ (� �� ������) ����� ������� ����
	if (down && ((count) == number))
	{
		glutPostRedisplay();
		start = true; //�������� ����
		mx = ax; //��������� ���������� ���������� ���� �� �
		my = ay; //��������� ���������� ���������� ���� �� �
		if (checkMouse(mx, my))
		{
			pp = true;
			if (count != 4)
				count++;
			else
				count = 1;
		}
	}
}

//�������� ������� ����������
 void display(void)
{
	for (i = 0; i < 11; i++)   // ���� ����� �������
	{
		for (j = 0; j < 11; j++) // ���� ������ ����
		{
			//��������� ��������� ������ �������
			if (start == false)
			{
				glColor3f(1.0f, 0.0f, 0.0f);   //���� ����� �������
				cells[0][ 0].condition = 1;
				glColor3f(0.0f, 1.0f, 0.0f);   //���� ����� �������
				cells[0][ 9].condition = 2;
				glColor3f(0.0f, 0.0f, 1.0f);   //���� ����� �����
				cells[9][ 0].condition = 3;
				glColor3f(1.0f, 1.0f, 0.0f);   //���� ����� ������
				cells[9][ 9].condition = 4;
				//if (i < 10 && j < 2)
				//{
				//    cells[i,j].condition = 1;
				//    cells[i,j+1].hline = 1;
				//}
				//if (i < 10 && j<10 && j>7)
				//{
				//    cells[i,j].condition = 2;
				//    cells[i,j - 1].hline = 1;
				//}
			}
			//����������� ������ ������
			if (i < 10 && j < 10) //�� ������ �� ��������� ����
			{
				//glColor3f(0.0f, 0.0f, 0.0f); //������ ����
				switch (cells[i][ j].condition)
				{
				case 1:
					glColor3f(1.0f, 0.0f, 0.0f);   //���� ����� �������
					drawcell(i, j);
					cnt1++;
					cells[i][ j].lock = true; //��������� ������ �� ������� ����
					cells[i][ j].hline = 0; //������� ���������
					break;
				case 2:
					glColor3f(0.0f, 1.0f, 0.0f);
					drawcell(i, j);
					cnt2++;
					cells[i][ j].lock = true; //��������� ������ �� ������� ����
					cells[i][ j].hline = 0; //������� ���������
					break;
				case 3:
					glColor3f(0.0f, 0.0f, 1.0f);
					drawcell(i, j);
					cnt3++;
					cells[i][ j].lock = true; //��������� ������ �� ������� ����
					cells[i][ j].hline = 0; //������� ���������
					break;
				case 4:
					glColor3f(1.0f, 1.0f, 0.0f);
					drawcell(i, j);
					cnt4++;
					cells[i][ j].lock = true; //��������� ������ �� ������� ����
					cells[i][ j].hline = 0; //������� ���������
					break;
				default: break;
				}
			}
			//������ �����
			drawline(i, j);
		}
	}
	//���������� ��� �������
	switch (count)
	{
	case 1:
		glColor3f(1.0f, 0.0f, 0.0f);   //���� ����� �������
		drawcell(14, 1);
		break;
	case 2:
		glColor3f(0.0f, 1.0f, 0.0f);   //���� ����� �������
		drawcell(14, 1);
		break;
	case 3:
		glColor3f(0.0f, 0.0f, 1.0f);   //���� ����� �������
		drawcell(14, 1);
		break;
	case 4:
		glColor3f(1.0f, 1.0f, 0.0f);   //���� ����� �������
		drawcell(14, 1);
		break;
	default: break;
	}

	switch (number)
	{
	case 1:
		glColor3f(1.0f, 0.0f, 0.0f);   //���� ����� �������
		drawcell(14, 3);
		break;
	case 2:
		glColor3f(0.0f, 1.0f, 0.0f);   //���� ����� �������
		drawcell(14, 3);
		break;
	case 3:
		glColor3f(0.0f, 0.0f, 1.0f);   //���� ����� �����
		drawcell(14, 3);
		break;
	case 4:
		glColor3f(1.0f, 1.0f, 0.0f);   //���� ����� ������
		drawcell(14, 3);
		break;
	default: break;
	}


	if (cnt1 + cnt2 == 100) //���� ����� ����
	{
		start = false;
		if (cnt1 > cnt2)
			glutSetWindowTitle("������� ��������");
		if (cnt1 < cnt2)
			glutSetWindowTitle("������� ��������");
		if (cnt1 == cnt2)
			glutSetWindowTitle("�����");
	}

	if (cnt1 + cnt2 != 100)
		cnt1 = cnt2 = 0; //�������� ��������
	glutSwapBuffers();
}

//��������� ���� OpenGL
 void init(void)
{
	 /* ��������� ������ ��� */
	 glClearColor(0.0, 0.0, 0.0, 0.0);
	 /* ������������� viewing values */
	 glMatrixMode(GL_PROJECTION);
	 glLoadIdentity();
	 glOrtho(0, 1000, 1000, 0, -1, 1);
}

//�������, �������� ��������
//void Timer()
//{
//    glutPostRedisplay();
//    glutTimerFunc(10, Timer, 0);
//}

//����� ������� ��������� ����
DWORD WINAPI Opengl()
{
	//glutTimerFunc(10, Timer, 1);
	glutMouseFunc(MousePressed);
	glutDisplayFunc(display);
	init();
	glutMainLoop();
	return 0; /* ISO C requires main to return int. */
}





int main(int argc, char* argv[])
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd != NULL)
	{
		ShowWindow(hwnd, SW_HIDE);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("CLIENT");


	char buff[1024];

	WSAData ws;
	WSAStartup(0x0202, &ws);

	SOCKET my_sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);

	HOSTENT *hst;
	// �������������� IP ������ �� ����������� � ������� ������
	if (inet_addr(SERVERADDR) != INADDR_NONE)
	{
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	}
	else if (hst = gethostbyname(SERVERADDR))
	{
		((unsigned long *)&dest_addr.sin_addr)[0] = ((unsigned long **)hst->h_addr_list)[0][0];
	}
	else
	{
		printf("Invalid address %s\n", SERVERADDR);
		closesocket(my_sock);
		WSACleanup();
		return -1;
	}

	if (connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr))!=0)
		return -1;
	else
	{
		printf("Connection successfully complited.\n\n");
		DWORD thID;
		//������� ����� ��� ������ � ��������
		CreateThread(NULL, NULL, ToServer, &my_sock, NULL, &thID);
		Opengl(); // ������� ���������
		return 0;
	}
	closesocket(my_sock);
	WSACleanup();
	return 0;
}

