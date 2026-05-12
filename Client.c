// 개발목적 : 컴퓨터공학부 대학교직원을 위한 온라인 학생정보관리 시스템 개발(client)
// 개발자 : 컴퓨터공학부 박교범
// 개발기간 : 26.05.10 ~ 26.05.17

#include "Common.h"

// 버퍼 클리어 함수: 입력 버퍼에 남아있는 개행 문자 제거
void clear_buffer() { while (getchar() != '\n'); }

// 배너 출력 함수: 시스템의 제목을 화면에 표시
void print_banner(const char* title) {
    printf("\n==============================================\n");
    printf(" %s\n", title);
    printf("==============================================\n");
}

// 메뉴 출력 함수: 사용자가 선택할 수 있는 기능들을 화면에 출력
void print_menu() {
    system("cls");
    print_banner("SMU 학생 정보 관리 시스템");
    printf("[1] 검색      [2] 등록      [3] 삭제      [4] 수정\n");
    printf("[5] 학과조회  [6] 교직원관리  [7] 저장      [8] 종료\n");
    printf("----------------------------------------------\n");
    printf("선택 > ");
}

// 엔터 대기 함수: 사용자에게 계속하려면 Enter 키를 누르도록 안내
void wait_enter() {
    clear_buffer(); // 입력 버퍼 클리어
    printf("\n계속하려면 Enter 키를 누르세요...");
    getchar();
}

// 메인 함수: 서버에 연결하여 로그인 후 메뉴를 통해 다양한 기능을 수행
int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { AF_INET, htons(PORT) };
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 서버에 연결 시도: 연결 실패 시 프로그램 종료
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return 1;

    // 로그인 기능: 교직원 계정으로 로그인해야만 이후 메뉴 사용이 가능하도록 구현
    Packet p; int logged_in = 0;
    system("cls"); // 화면 초기화
    print_banner("SMU 학생 정보 관리 시스템 - 로그인");

    // 로그인 루프: 로그인 성공 시 메뉴로 이동, 실패 시 재시도
    while (!logged_in) {
        char id[20], pw[20];
        printf("%s ID: ", ICON_USER); scanf("%s", id);
        printf("%s PW: ", ICON_USER); scanf("%s", pw);
        p.cmdCode = CMD_LOGIN; sprintf(p.data, "%s %s", id, pw);
        send(s, (char*)&p, sizeof(Packet), 0);
        recv(s, (char*)&p, sizeof(Packet), 0);
        if (p.statusCode == ST_SUCCESS) {
            printf("\n%s\n", p.data);
            printf("%s 로그인에 성공했습니다. 메뉴로 이동합니다.\n", ICON_SUCCESS);
            logged_in = 1;
            wait_enter();
        } else {
            printf("\n%s 로그인 실패! 다시 시도하세요.\n", ICON_WARN);
        }
    }

    system("cls");

    // 요구사항별 메뉴 루프: 사용자가 선택한 기능을 서버로 전달하고 결과 받기
    // 메뉴 루프: 사용자가 1~8 사이의 숫자를 입력하여 기능 선택, 8 입력 시 종료
    while (1) {
        print_menu();
        int menu; if (scanf("%d", &menu) != 1) { clear_buffer(); continue; }

        if (menu == 8) break;
        if (menu == 1) p.cmdCode = CMD_SEARCH;
        else if (menu == 2) p.cmdCode = CMD_ADD;
        else if (menu == 3) p.cmdCode = CMD_DELETE;
        else if (menu == 4) p.cmdCode = CMD_MODIFY;
        else if (menu == 5) p.cmdCode = CMD_DEPT_LIST;
        else if (menu == 6) p.cmdCode = CMD_USER_MGMT;
        else if (menu == 7) p.cmdCode = CMD_SAVE;
        else {
            printf("\n%s 잘못된 선택입니다. 1~8 사이의 숫자를 입력하세요.\n", ICON_WARN);
            wait_enter();
            continue;
        }

        if (menu == 1) {    // 검색 기능: 이름과 학번을 입력받아 서버로 전송
            char name[20], id[15];
            printf("\n[학생 검색]\n이름과 학번 입력: "); scanf("%s %s", name, id);
            sprintf(p.data, "%s %s", name, id);
        } else if (menu == 4) {     // 수정 기능: 기존 학생 정보와 새 정보를 입력받아 서버로 전송
            char oldName[20], oldId[15];
            char newName[20], newId[15], newDept[30], newEmail[40], newPhone[15];
            int newYear; float newGpa;
            printf("\n[학생 정보 수정]\n수정할 학생 이름과 학번 입력: "); scanf("%s %s", oldName, oldId);
            printf("새 정보 입력(이름 학번 학과 학년(1-4) 평점(0-4.5) 이메일 전화)\n> ");
            scanf("%s %s %s %d %f %s %s", newName, newId, newDept, &newYear, &newGpa, newEmail, newPhone);
            if (newYear < 1 || newYear > 4 || newGpa < 0 || newGpa > 4.5) {     // 입력 유효성 검사: 학년은 1~4, 평점은 0~4.5 범위 내에 있어야 함
                printf("\n%s 입력 범위가 올바르지 않습니다. 다시 시도하세요.\n", ICON_WARN);
                wait_enter();
                continue;
            }
            sprintf(p.data, "%s %s %s %s %s %d %.2f %s %s",
                oldName, oldId, newName, newId, newDept, newYear, newGpa, newEmail, newPhone);
        } else if (menu == 6) {   // 교직원 관리 기능: 신규 직원 추가 또는 기존 직원 삭제
            int sub;
            printf("\n[교직원 관리]\n[1] 신규 직원 추가 [2] 직원 삭제\n선택 > ");
            scanf("%d", &sub);
            if (sub == 1) {     // 신규 직원 추가
                char id[20], pw[20], dept[30], name[20];
                printf("새 직원 ID PW 부서 이름\n> ");
                scanf("%s %s %s %s", id, pw, dept, name);
                sprintf(p.data, "ADD %s %s %s %s", id, pw, dept, name);
            } else if (sub == 2) {      // 직원 삭제, 삭제 시 확인 절차 추가
                char id[20], pw[20], confirm[8];
                printf("삭제할 교직원 ID PW\n> ");
                scanf("%s %s", id, pw);
                printf("정말 삭제하시겠습니까? (YES/NO) ");
                scanf("%s", confirm);
                if (strcmp(confirm, "YES") != 0) {
                    printf("\n%s 삭제가 취소되었습니다.\n", ICON_WARN);
                    wait_enter();
                    continue;
                }
                sprintf(p.data, "DEL %s %s", id, pw);
            } else {
                printf("\n%s 잘못된 선택입니다.\n", ICON_WARN);
                wait_enter();
                continue;
            }
        } else if (menu == 7) {     // 저장 기능: 서버에 저장 명령을 보내어 현재 데이터를 파일로 저장(입력 데이터 필요X)
            p.data[0] = '\0';
        } else if (menu == 3) {     // 삭제 기능: 이름과 학번을 입력받아 서버로 전송하여 해당 학생 정보 삭제
            char name[20], id[15];
            printf("\n[학생 삭제]\n삭제할 이름과 학번 입력: "); scanf("%s %s", name, id);
            sprintf(p.data, "%s %s", name, id);
        } else if (menu == 2) {     // 등록 기능: 학생 정보를 입력받아 서버로 전송하여 새로운 학생 정보 추가, 입력 유효성 검사 포함 
            char n[20], id[15], d[30], em[40], ph[15]; int y; float g;
            printf("\n[학생 등록]\n이름 학번 학과 학년(1-4) 학점(0-4.5) 이메일 전화\n> ");
            scanf("%s %s %s %d %f %s %s", n, id, d, &y, &g, em, ph);
            if (y < 1 || y > 4 || g < 0 || g > 4.5) { 
                printf("\n%s 범위 오류! 다시 입력하세요.\n", ICON_WARN);
                wait_enter();
                continue; 
            }
            sprintf(p.data, "%s %s %s %d %.2f %s %s", n, id, d, y, g, em, ph);
        } else {        // 학과 조회 기능: 학과 이름을 입력받아 서버로 전송하여 해당 학과에 속한 학생 리스트 출력
            printf("\n학과 입력: "); scanf("%s", p.data);
        }

        send(s, (char*)&p, sizeof(Packet), 0);
        recv(s, (char*)&p, sizeof(Packet), 0);
        
        if (p.statusCode == ST_SUCCESS) printf("\n%s\n%s\n", ICON_SUCCESS, p.data);
        else printf("\n%s 처리 실패\n", ICON_WARN);
        wait_enter();
    }

    // 프로그램 종료: 소켓 닫기 및 WSA 정리
    closesocket(s); WSACleanup(); return 0;
}