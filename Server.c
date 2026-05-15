// 개발목적 : 컴퓨터공학부 대학교직원을 위한 온라인 학생정보관리 시스템 개발(server)
// 개발자 : 컴퓨터공학부 박교범
// 개발기간 : 26.05.10 ~ 26.05.17

#include "Common.h"
#include <process.h>
// 학생 정보 연결 리스트의 헤드 포인터
Student* student_head = NULL;
// 교직원 정보 연결 리스트의 헤드 포인터
User* user_head = NULL;
// 학생과 교직원 데이터에 대한 동시 접근을 제어하기 위한 크리티컬 섹션
CRITICAL_SECTION cs_student, cs_user;

// 1. 프로그램 시작 시 저장된 파일에서 데이터를 메모리 연결 리스트로 읽어옴
void load_all_data() {
    FILE* fs = fopen("SMU_Students.txt", "r");                                                                      // 학생 정보 파일 열기
    if (fs) {
        Student s;
        while (fscanf(fs, "%s %s %s %d %f %s %s", s.name, s.id, s.dept, &s.year, &s.gpa, s.email, s.phone) == 7) {  // 파일에서 학생 정보 읽기
            Student* n = (Student*)malloc(sizeof(Student));                                                         // 새 학생 노드 할당
            memcpy(n, &s, sizeof(Student));                                                                         // 읽은 정보로 노드 초기화
            n->next = student_head; student_head = n;                                                               // 연결 리스트에 노드 추가
        }
        fclose(fs);                                                                                                 // 파일 닫기
    }
    FILE* fu = fopen("Users.txt", "r");                                                         // 교직원 정보 파일 열기
    if (fu) {
        User u;
        while (fscanf(fu, "%s %s %s %s", u.id, u.pw, u.dept, u.name) == 4) {                    // 파일에서 교직원 정보 읽기
            User* n = (User*)malloc(sizeof(User));                                              // 새 교직원 노드 할당
            memcpy(n, &u, sizeof(User));                                                        // 읽은 정보로 노드 초기화
            n->next = user_head; user_head = n;                                                 // 연결 리스트에 노드 추가
        }
        fclose(fu);                                                                             // 파일 닫기 
    }
}

// 비교 함수: 학생 이름 + 학번 순 정렬
int compare_students(const void* a, const void* b) {
    Student* s1 = *(Student**)a;
    Student* s2 = *(Student**)b;
    int nameCmp = strcmp(s1->name, s2->name);
    if (nameCmp != 0) return nameCmp;
    return strcmp(s1->id, s2->id);
}

// 비교 함수: 교직원 이름 + ID 순 정렬
int compare_users(const void* a, const void* b) {
    User* u1 = *(User**)a;
    User* u2 = *(User**)b;
    int nameCmp = strcmp(u1->name, u2->name);
    if (nameCmp != 0) return nameCmp;
    return strcmp(u1->id, u2->id);
}

// 학생 데이터를 이름 가나다 순으로 정렬해서 안전하게 파일에 저장
int save_student_file() {
    EnterCriticalSection(&cs_student);                                  // 임계 구역 진입
    int success = 1;
    int count = 0;
    for (Student* c = student_head; c; c = c->next) count++;            // 학생 수 계산
    if (count > 0) {
        Student** arr = (Student**)malloc(sizeof(Student*) * count);    // 포인터 배열 생성
        Student* c = student_head;
        for (int i = 0; i < count; i++) {                               // 배열에 포인터 저장
            arr[i] = c;
            c = c->next;
        }
        qsort(arr, count, sizeof(Student*), compare_students);          // 이름 가나다 순 정렬
        FILE* ft = fopen("SMU_Students_temp.txt", "w");
        if (ft) {                                                       // 임시 파일에 저장
            for (int i = 0; i < count; i++) {                           // 정렬된 순서로 학생 정보 저장
                Student* s = arr[i];
                fprintf(ft, "%s\t%s\t%s\t%d\t%.2f\t%s\t%s\n",           // 탭으로 구분된 형식으로 저장
                    s->name, s->id, s->dept, s->year, s->gpa, s->email, s->phone);
            }
            fclose(ft);
            remove("SMU_Students.txt");                                 // 기존 파일 삭제
            rename("SMU_Students_temp.txt", "SMU_Students.txt");        // 임시 파일을 정식 파일로 이름 변경
        } else {
            success = 0;
        }
        free(arr);                                                      // 포인터 배열 메모리 해제
    } else {                                                            // 학생이 없는 경우 빈 파일로 저장
        FILE* ft = fopen("SMU_Students_temp.txt", "w");
        if (ft) {
            fclose(ft);
            remove("SMU_Students.txt");
            rename("SMU_Students_temp.txt", "SMU_Students.txt");        // 기존 파일 삭제 후 임시 파일을 정식 파일로 이름 변경
        } else {
            success = 0;
        }
    }
    LeaveCriticalSection(&cs_student);                                  // 임계 구역 해제
    return success;                                                     // 저장 성공 여부 반환
}

// 교직원 데이터를 이름 가나다 순으로 정렬해서 안전하게 파일에 저장
int save_user_file() {
    EnterCriticalSection(&cs_user);
    int success = 1;
    int count = 0;
    for (User* u = user_head; u; u = u->next) count++;                  // 교직원 수 계산
    if (count > 0) {
        User** arr = (User**)malloc(sizeof(User*) * count);             // 포인터 배열 생성
        User* u = user_head;                                            
        for (int i = 0; i < count; i++) {                               // 배열에 포인터 저장
            arr[i] = u;
            u = u->next;
        }
        qsort(arr, count, sizeof(User*), compare_users);                // 이름 가나다 순 정렬
        FILE* ft = fopen("Users_temp.txt", "w");                        // 임시 파일에 저장
        if (ft) {
            for (int i = 0; i < count; i++) {                           // 정렬된 순서로 교직원 정보 저장
                User* user = arr[i];
                fprintf(ft, "%s\t%s\t%s\t%s\n",                         // 탭으로 구분된 형식으로 저장
                    user->id, user->pw, user->dept, user->name);
            }
            fclose(ft);
            remove("Users.txt");
            rename("Users_temp.txt", "Users.txt");                      // 기존 파일 삭제 후 임시 파일을 정식 파일로 이름 변경
        } else {
            success = 0;
        }
        free(arr);                                                      // 포인터 배열 메모리 해제
    } else {                                                            // 교직원이 없는 경우 빈 파일로 저장
        FILE* ft = fopen("Users_temp.txt", "w");
        if (ft) {
            fclose(ft);
            remove("Users.txt");
            rename("Users_temp.txt", "Users.txt");
        } else {
            success = 0;                                                // 저장 실패 시 0 반환
        }
    }
    LeaveCriticalSection(&cs_user);                                     // 임계 구역 해제
    return success;                                                     // 저장 성공 여부 반환
}

// 2. 학생 정보와 교직원 정보를 한꺼번에 파일로 저장
int save_all_data() {
    int ok1 = save_student_file();
    int ok2 = save_user_file();
    return ok1 && ok2;
}

// 3. 서버가 24시간 동작 중일 때 매일 새벽 01시에 자동으로 저장하는 스레드
unsigned __stdcall autosave_thread(void* arg) {
    int saved_today = 0;
    while (1) {
        time_t now = time(NULL);
        struct tm* current = localtime(&now);                   // 현재 시간 정보 가져오기
        if (current->tm_hour == 1 && current->tm_min == 0) {    // 매일 새벽 01:00에 자동 저장
            if (!saved_today) {
                save_all_data();                                // 자동 저장 수행
                saved_today = 1;                                // 오늘 저장 완료 표시
            }
        } else {
            saved_today = 0;                                    // 시간이 01:00이 아닐 때 저장 플래그 초기화
        }
        Sleep(60000);                                           // 1분마다 시간 체크
    }
    return 0;
}

// 4. 클라이언트 요청을 처리하는 스레드 함수
//    사용자 로그인, 학생 정보 검색/추가/삭제/수정/학과별조회, 교직원 관리(신규추가/삭제), 저장, 종료의 기능을 수행
unsigned __stdcall handle_client(void* arg) {                                           // 클라이언트 요청 처리 (S002 ~ S007)
    SOCKET s = (SOCKET)arg;                                                             // 클라이언트 소켓
    Packet req, res;                                                                    // 요청과 응답 패킷 구조체
    while (recv(s, (char*)&req, sizeof(Packet), 0) > 0) {                               // 클라이언트로부터 요청 패킷 수신
        memset(&res, 0, sizeof(Packet));                                                // 응답 패킷 초기화
        res.msgType = 1; res.cmdCode = req.cmdCode; res.statusCode = ST_FAIL;           // 응답 패킷 기본값 설정 (응답 유형, 명령 코드, 실패 상태)

        switch (req.cmdCode) {                                                          // 명령 코드에 따른 요청 처리
            case CMD_LOGIN: {                                                           // S002 로그인
                char id[20], pw[20]; sscanf(req.data, "%s %s", id, pw);                 // 요청 데이터에서 ID와 패스워드 추출
                for (User* u = user_head; u; u = u->next) {                             // 교직원 리스트에서 ID와 패스워드 일치하는 사용자 검색
                    if (!strcmp(u->id, id) && !strcmp(u->pw, pw)) {                     // 로그인 성공 시 응답 패킷에 성공 상태와 환영 메시지 설정
                        res.statusCode = ST_SUCCESS;
                        sprintf(res.data, "%s(%s)님 로그인 성공", u->name, u->dept); break;
                    }
                }
                break;
            }
            case CMD_SEARCH: {                                                          // [1]검색(S006) 
                char name[20], id[15];
                if (sscanf(req.data, "%s %s", name, id) == 2) {                         // 요청 데이터에서 이름과 학번 추출
                    for (Student* c = student_head; c; c = c->next) {                   // 학생 리스트에서 이름과 학번 일치하는 학생 검색
                        if (!strcmp(c->name, name) && !strcmp(c->id, id)) {             // 검색 성공 시 응답 패킷에 학생 정보 설정
                            sprintf(res.data,                                           // 학생 정보 포맷으로 응답 데이터 설정
                                "이름: %s\n학번: %s\n학과: %s\n학년: %d\n학점: %.2f\n이메일: %s\n전화: %s",
                                c->name, c->id, c->dept, c->year, c->gpa, c->email, c->phone);
                            res.statusCode = ST_SUCCESS; break;                         
                        }
                    }
                }
                break;
            }
            case CMD_ADD: {                                                             // [2]등록 기능(S003)
                char n[20], id[15], d[30], em[40], ph[15]; int y; float g;
                sscanf(req.data, "%s %s %s %d %f %s %s", n, id, d, &y, &g, em, ph);

                int exists = 0;
                EnterCriticalSection(&cs_student);                                      // 학생 리스트에 동시 접근 방지 위해 임계 구역 진입
                
                // 중복 검사: 이름과 학번이 모두 동일한 학생이 있는지 확인
                for (Student* c = student_head; c; c = c->next) {
                    if (!strcmp(c->name, n) && !strcmp(c->id, id)) {
                        exists = 1;
                        break;
                    }
                }

                if (exists) {
                    res.statusCode = ST_FAIL;
                    strcpy(res.data, "이미 등록된 동일한 학생(이름/학번)이 존재합니다.");
                } else {
                    // 중복이 없을 때만 메모리 할당 및 추가
                    Student* ns = (Student*)malloc(sizeof(Student));
                    strcpy(ns->name, n); strcpy(ns->id, id); strcpy(ns->dept, d);
                    ns->year = y; ns->gpa = g;
                    strcpy(ns->email, em); strcpy(ns->phone, ph);
                    
                    ns->next = student_head; 
                    student_head = ns;
                    
                    res.statusCode = ST_SUCCESS; 
                    strcpy(res.data, "학생 등록 완료");                                 // 등록 성공 시 응답 패킷에 성공 상태와 완료 메시지 설정
                }
                LeaveCriticalSection(&cs_student);                                      // 임계 구역 해제
                break;
            }
            case CMD_DELETE: {                                                          // [3]삭제 기능(S003)
                char name[20], id[15];
                sscanf(req.data, "%s %s", name, id);                                    // 요청 데이터에서 이름과 학번 추출
                EnterCriticalSection(&cs_student);                                      // 학생 리스트에 동시 접근 방지 위해 임계 구역 진입
                Student* prev = NULL;
                for (Student* c = student_head; c; c = c->next) {                       // 학생 리스트 순회
                    if (!strcmp(c->name, name) && !strcmp(c->id, id)) {                 // 삭제할 학생을 찾은 경우
                        if (prev) prev->next = c->next;                                 // 이전 노드가 있는 경우 이전 노드의 next를 현재 노드의 next로 연결
                        else student_head = c->next;                                    // 삭제할 학생이 리스트의 헤드인 경우 헤드를 다음 노드로 변경
                        free(c);                                                        // 삭제할 학생 노드 메모리 해제
                        res.statusCode = ST_SUCCESS;                                    // 삭제 성공 시 응답 패킷에 성공 상태와 완료 메시지 설정
                        strcpy(res.data, "학생 삭제 완료");
                        break;
                    }
                    prev = c;                                                           // 다음 노드로 이동
                }
                LeaveCriticalSection(&cs_student);                                      // 학생 리스트에 대한 임계 구역 해제
                break;
            }
            case CMD_MODIFY: {                                                          // [4]수정 기능(S006)
                char oldName[20], oldId[15];                                            // 기존 학생 정보
                char newName[20], newId[15], newDept[30], newEmail[40], newPhone[15];   // 새로운 학생 정보
                int newYear; float newGpa;                                              // 새로운 학생 정보 중 학년과 학점
                if (sscanf(req.data, "%s %s %s %s %s %d %f %s %s",                      // 요청 데이터에서 기존 학생 정보와 새로운 학생 정보 추출
                    oldName, oldId, newName, newId, newDept, &newYear, &newGpa, newEmail, newPhone) == 9) {
                    EnterCriticalSection(&cs_student);                                  // 학생 리스트에 동시 접근 방지 위해 임계 구역 진입
                    Student* c = student_head;                                          
                    while (c) {                                                         // 수정할 학생을 찾을 때까지 순회
                        if (!strcmp(c->name, oldName) && !strcmp(c->id, oldId)) {       // 수정할 학생을 찾은 경우
                            strcpy(c->name, newName);                                   // 학생 정보 수정
                            strcpy(c->id, newId);
                            strcpy(c->dept, newDept);
                            c->year = newYear;
                            c->gpa = newGpa;
                            strcpy(c->email, newEmail);
                            strcpy(c->phone, newPhone);
                            res.statusCode = ST_SUCCESS;                                // 수정 성공 시 응답 패킷에 성공 상태와 수정된 학생 정보 설정
                            sprintf(res.data,                                           // 수정된 학생 정보 포맷으로 응답 데이터 설정
                                "학생 정보 수정 완료: %s %s %s %d %.2f %s %s",
                                c->name, c->id, c->dept, c->year, c->gpa, c->email, c->phone);
                            break;
                        }
                        c = c->next;                                                    // 다음 노드로 이동
                    }
                    LeaveCriticalSection(&cs_student);                                  // 학생 리스트에 대한 임계 구역 해제
                    if (res.statusCode != ST_SUCCESS) {                                 // 수정할 학생을 찾지 못한 경우 응답 패킷에 실패 상태와 오류 메시지 설정
                        res.statusCode = ST_FAIL;
                        strcpy(res.data, "수정할 학생을 찾을 수 없습니다");
                    }
                } else {                                                                // 데이터 형식이 잘못된 경우 응답 패킷에 실패 상태와 오류 메시지 설정
                    res.statusCode = ST_FAIL;
                    strcpy(res.data, "잘못된 수정 요청 형식입니다");
                }
                break;
            }
            case CMD_DEPT_LIST: {                                                                   // [5]학과별 조회 기능(S007) 
                char buf[128];                                                                      // 응답 데이터 버퍼
                for (Student* c = student_head; c; c = c->next) {
                    if (strstr(c->dept, req.data)) {                                                // 학생의 학과에 요청 데이터(학과명)가 포함된 경우
                        sprintf(buf, "[%s] %s (%d학년)\n", c->id, c->name, c->year);                // 학생 정보를 ID, 이름, 학년 형식으로 버퍼에 저장
                        if (strlen(res.data) + strlen(buf) < MAX_DATA_SIZE) strcat(res.data, buf);  // 응답 데이터에 학생 정보 추가 (버퍼 크기 초과 방지)
                        res.statusCode = ST_SUCCESS;
                    }
                }
                break;
            }
            case CMD_USER_MGMT: {                                                           // [6]교직원 관리 기능(S004)
                char op[8], id[20], pw[20], dept[30], name[20];
                int count = sscanf(req.data, "%s %s %s %s %s", op, id, pw, dept, name);     // 요청 데이터에서 교직원 관리 작업 유형과 교직원 정보 추출
                if (!strcmp(op, "ADD") && count == 5) {                                     // 신규 교직원 추가인 경우
                    int exists = 0;
                    for (User* u = user_head; u; u = u->next) {                             // 교직원 리스트에서 ID가 이미 존재하는지 확인
                        if (!strcmp(u->id, id)) {
                            exists = 1;
                            break;
                        }
                    }
                    if (exists) {                                                           // ID가 이미 존재하는 경우 응답 패킷에 실패 상태와 오류 메시지 설정
                        res.statusCode = ST_FAIL;
                        strcpy(res.data, "이미 존재하는 사용자입니다");
                    } else {                                                                // ID가 존재하지 않는 경우 새 교직원 노드 생성 및 리스트에 추가
                        User* nu = (User*)malloc(sizeof(User));
                        strcpy(nu->id, id);
                        strcpy(nu->pw, pw);
                        strcpy(nu->dept, dept);
                        strcpy(nu->name, name);
                        EnterCriticalSection(&cs_user);                                     // 교직원 리스트에 동시 접근 방지 위해 임계 구역 진입
                        nu->next = user_head; user_head = nu;                               // 새 교직원 노드를 교직원 리스트의 헤드에 추가
                        LeaveCriticalSection(&cs_user);                                     // 임계 구역 해제
                        res.statusCode = ST_SUCCESS;
                        strcpy(res.data, "교직원 추가 완료");
                    }
                } else if (!strcmp(op, "DEL") && count == 3) {                              // 교직원 삭제인 경우
                    int deleted = 0;
                    EnterCriticalSection(&cs_user);                                         // 교직원 리스트에 동시 접근 방지 위해 임계 구역 진입
                    User* prev = NULL;
                    for (User* u = user_head; u; u = u->next) {                             // 교직원 리스트 순회
                        if (!strcmp(u->id, id) && !strcmp(u->pw, pw)) {                     // 삭제할 교직원을 찾은 경우
                            if (prev) prev->next = u->next;
                            else user_head = u->next;
                            free(u);                                                        // 삭제할 교직원 노드 메모리 해제
                            deleted = 1;                                                    // 삭제 성공 표시
                            break;
                        }
                        prev = u;
                    }
                    LeaveCriticalSection(&cs_user);                                         // 교직원 리스트에 대한 임계 구역 해제
                    if (deleted) {                                                          // 삭제 성공 시 응답 패킷에 성공 상태와 완료 메시지 설정
                        res.statusCode = ST_SUCCESS;
                        strcpy(res.data, "교직원 삭제 완료");
                    } else {                                                                // 삭제할 교직원을 찾지 못한 경우 응답 패킷에 실패 상태와 오류 메시지 설정
                        res.statusCode = ST_FAIL;
                        strcpy(res.data, "삭제할 교직원을 찾을 수 없습니다");
                    }
                } else {                                                                    // 잘못된 요청인 경우 응답 패킷에 실패 상태와 오류 메시지 설정
                    res.statusCode = ST_FAIL;
                    strcpy(res.data, "잘못된 교직원 관리 요청입니다");
                }
                break;
            }
            case CMD_SAVE: {                                                          // [7]저장 기능(수동 저장기능)
                if (save_all_data()) {                                                // 저장 성공 시 응답 패킷에 성공 상태와 완료 메시지 설정
                    res.statusCode = ST_SUCCESS;
                    strcpy(res.data, "데이터 저장 완료");
                } else {                                                              // 저장 실패 시 응답 패킷에 실패 상태와 오류 메시지 설정
                    res.statusCode = ST_FAIL;
                    strcpy(res.data, "데이터 저장 실패");
                }
                break;
            }
            
            case CMD_EXIT: closesocket(s); return 0;                                    // [8]종료 기능(S008) - 클라이언트 소켓 닫고 스레드 종료
        }
        send(s, (char*)&res, sizeof(Packet), 0);                                        // 처리된 요청에 대한 응답 패킷을 클라이언트로 전송
    }
    return 0;
}

// 5. 서버 메인 함수: 크리티컬 섹션 초기화, 데이터 로드, 백업 스레드 시작, 소켓 생성 및 바인딩, 클라이언트 연결 대기 및 요청 처리 스레드 생성
int main() {
    InitializeCriticalSection(&cs_student); InitializeCriticalSection(&cs_user);
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    load_all_data();
    
    // 백업 스레드 시작
    _beginthreadex(NULL, 0, autosave_thread, NULL, 0, NULL);

    // 소켓 생성 및 바인딩
    SOCKET l = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = { AF_INET, htons(PORT), INADDR_ANY };
    bind(l, (struct sockaddr*)&a, sizeof(a));
    listen(l, 5);

    printf("%s 서버 구동 중...\n", ICON_SUCCESS);
    // 클라이언트 연결 대기 및 요청 처리 스레드 생성
    while (1) {
        SOCKET c = accept(l, NULL, NULL);                           // 클라이언트 연결 수락
        _beginthreadex(NULL, 0, handle_client, (void*)c, 0, NULL);  // 클라이언트 요청 처리 스레드 생성
    }
    return 0;
}