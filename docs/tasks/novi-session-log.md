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

## 2026-11-08 재개

### 완료

- tuple equality의 잘못된 역참조를 고치고 parser의 try span과 `console.print` callback 컴파일 오류를 수정했다.
- `break`/`continue`는 현재 루프가 실제로 빠져나가는 try 범위의 finalizer만 실행하도록 컴파일 경로를 고쳤다. 정리 코드를 실행하기 전에 나가는 exception handler를 pop해 finalizer 오류가 바깥 handler로 전달되게 했다.
- catch 환경을 handler 종료 시 원래 환경으로 복원해 catch 변수 범위가 밖으로 새지 않게 했다.
- CTest 등록과 tuple equality, finally의 break/continue/return/throw, catch scope를 덮는 8개 회귀 사례를 추가했다.
- README 상태를 `개발 재개 (2026-11-08)`로 바꾸고 직접 빌드 명령에 `project.cpp`를 포함했다.

### 검증 결과와 환경

- 기본 `clang++` 명령은 CLT의 기본 include 경로에 C++ 표준 헤더가 없어 실패했다. SDK의 libc++ 경로를 `-isystem "$(xcrun --show-sdk-path)/usr/include/c++/v1"`로 지정해 C++17 빌드를 수행했다.
- 위 직접 빌드는 warning 없이 성공했고, 새 회귀 사례 8개와 `examples/*.nvi` 실행이 성공했다.
- 로컬에는 `cmake`/`ctest` 실행 파일이 없어 CMake configure와 CTest suite 자체는 실행하지 못했다. CTest 등록은 저장소에 추가했다.
- Rust와 asm은 이 안정화 변경에 사용하지 않았다. 병목 측정이 없는 상태에서 도입해도 최적화 효과를 입증할 수 없어서다.

### 다음 작업

1. CMake가 설치된 CI나 개발 환경에서 configure/build/CTest를 실행해 CTest 등록까지 확인한다.
2. catch 변수와 lexical scope, 타입 오류, 미정의 이름을 컴파일 전에 진단할 semantic analyzer의 범위를 정하고 구현한다.
3. 작은 VM benchmark를 추가한 뒤 실제 병목만 대상으로 C++ 최적화를 검토한다. Rust/asm은 C++ baseline보다 유의미한 이득이 측정되는 경로에만 고려한다.
