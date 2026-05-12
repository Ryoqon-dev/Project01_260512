// 개발목적 : 컴퓨터공학부 대학교직원을 위한 온라인 학생정보관리 시스템 개발(Common.h)
// 개발자 : 컴퓨터공학부 박교범
// 개발기간 : 26.05.10 ~ 26.05.17

#ifndef COMMON_H
#define COMMON_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <windows.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 55555
#define MAX_DATA_SIZE 4096 

// UI 아이콘
#define ICON_USER    " [환영] "
#define ICON_WARN    " [경고] "
#define ICON_SUCCESS " [완료] "

// 상태 코드
typedef enum { 
    ST_SUCCESS = 0, ST_FAIL = 1, ST_AUTH_FAIL = 2, ST_NO_DATA = 3 
} StatusCode;

// 명령 코드
typedef enum { 
    CMD_LOGIN = 1, CMD_SEARCH = 2, CMD_ADD = 3, CMD_DELETE = 4, 
    CMD_MODIFY = 5, CMD_DEPT_LIST = 6, CMD_USER_MGMT = 7, CMD_SAVE = 8, CMD_EXIT = 9 
} CommandCode;

// 데이터 송수신용 구조체
typedef struct {
    int msgType;     // 0: 요청, 1: 응답
    int cmdCode;     // 기능 구분
    int dataLen;     // 데이터 크기
    int statusCode;  // 성공/실패 여부
    char data[MAX_DATA_SIZE];   // 실제 데이터는 최대 4096바이트로 제한
} Packet;

// 학생 정보
typedef struct Student {
    char name[20], id[15], dept[30], email[40], phone[15];
    int year;
    float gpa;
    struct Student* next;
} Student;

// 교직원 정보 
typedef struct User {
    char id[20], pw[20], dept[30], name[20];
    struct User* next;
} User;

#endif