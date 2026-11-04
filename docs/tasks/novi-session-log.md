# Novi 작업 재개 기록

## 2026-11-05

### 오늘 반영하고 푸시한 내용

- 예외 구문에 `finally`를 추가하고 compiler/VM 예외 처리 경로에 연결했다.
- 문자열 인덱싱, `.length()`, `foreach`를 UTF-8 코드포인트 기준으로 처리한다.
- 불변 tuple의 생성자 `tuple(...)`, 타입 검사, 인덱싱, 길이, 순회, 비교를 추가했다.
- list API에 `contains`, `remove`, `clear`, `insert`, `extend`, `pop()` 및 `pop(index)`를 추가했다.
- set에 `remove`, `clear`; dictionary에 `contains`, `remove`, `clear`, `length`; queue에 `peek`, `clear`를 추가했다.
- 예제와 언어 기능 문서를 갱신했다.

현재 `main`의 마지막 푸시 커밋은 `eac09c5` (`Allow indexed list pop in Novi`)다. 그 직전 관련 작업 커밋은 `c007159`, `db3413c`, `be2193f`, `a3a7348`, `482dc78`, `2bee41a`, `d273a99`, `8377734`, `5aa3e1c`, `db0e4c5`, `41c6551`이다.

### 검증 상태

- `git diff --check`는 각 변경 단위에서 실행했다.
- 빌드와 런타임 테스트는 실행하지 않았다. 따라서 위 기능은 아직 실행 검증 완료 상태가 아니다.
- 정적 코드 검토에서 `src/novi/runtime.cpp`의 tuple 비교 코드에 잘못된 포인터 역참조가 있어 컴파일을 막을 가능성을 확인했다.
- `src/novi/compiler.cpp`에서 `break`/`continue`가 현재 활성화된 모든 `finally`를 실행하도록 되어 있다. try 블록 안쪽 루프만 빠져나가는 경우에도 `finally`를 일찍 실행할 수 있으므로 제어 흐름을 수정해야 한다.

### 다음 작업 시작 지점

1. `src/novi/runtime.cpp`의 tuple equality 포인터 비교와 멤버 접근을 수정한다.
2. `finally` 실행 범위를 제어 흐름 기준으로 고쳐, try를 실제로 벗어날 때만 실행되게 한다.
3. Novi를 빌드해 현재 변경의 컴파일 오류를 복구한다.
4. lexer/parser/compiler/VM 기능에 회귀 테스트 체계를 추가하고, 앞서 추가한 예외·문자열·컬렉션 동작을 검증한다.
5. 이후 P0의 의미 분석과 함수/타입 정합성 작업을 이어간다.

다음 세션은 기능 추가보다 1~4번의 안정화부터 시작한다.
