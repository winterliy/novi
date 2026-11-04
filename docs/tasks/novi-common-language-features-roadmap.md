# Novi 보편 언어 기능 확장 로드맵

## 목표

일반 목적 언어에서 반복해서 쓰이는 기능을 Novi에 단계적으로 갖춘다. 기존 Novi 문법과 사용자가 정한 규칙을 먼저 따른다. 이미 표기가 정해졌다면 그 표기를 구현하고, 표기가 비어 있으면 Novi의 문장 종결(`;`), 블록(`{}`), 타입 선언, 키워드 별칭 규칙을 우선 적용한 문법 후보를 제안한다. 제안 표기는 사용자가 결정하기 전에는 확정 문법으로 취급하지 않는다.

Novi의 실행 성능 최적화 목표를 유지한다. 기능 수를 늘리는 것보다 실행 모델, 메모리 비용, 오류 처리를 측정 가능하게 설계한다.

## 상태 표기

- **합의됨**: 사용자 표기/규칙이 문서에 기록되어 있음.
- **구현 일부**: lexer/parser/VM 일부가 동작하지만 전체 의미 규칙 또는 값 종류가 미완성.
- **미구현**: 표기만 합의되었거나 설계 문서만 있음.
- **제안 필요**: 표기 또는 의미 규칙을 사용자가 아직 결정하지 않음.

## 현재 확인한 기반

현재 코드에는 lexer, recursive-descent parser, AST, 바이트코드 compiler, stack VM이 있다. 소스 코드는 `.nvi`이며, `vari`/`let`/기존 변수 freeze, 함수·클래스 일부, 동기 조건/루프, `foreach`와 정수 `range`, 출력, 캐스트, REPL 및 `novi new` 프로젝트 scaffold가 있다. `range`는 지연 수열이며 종착값 제외, 0이 아닌 정수 간격을 사용한다. P0 시작 구현은 dictionary·list/set/queue의 기본 API, `and`/`or` 단락 평가, 선언 타입과 재대입 타입 검사를 포함한다. 이는 아직 빌드나 런타임 검증을 수행한 상태를 뜻하지 않는다.

파서/VM 지원 여부는 기능별로 다르므로 문서의 `implemented` 상태는 소스 구조에 대한 이번 작업 시점의 확인이며, 런타임 적합성 테스트 결과를 의미하지 않는다.

## 기능 분류와 Novi 표기 초안

| 영역 | Novi 표기/방향 | 상태 | 우선순위 |
|---|---|---:|---:|
| 값, 변수, 상수, 산술·논리·비트 연산 | `vari`, `let`, `constant`, 합의된 연산자 | 구현 일부 | P0 |
| 조건과 반복 | `is/also/isnothing`, 별칭 `if/elif/else`, `during`, `reflex`, `foreach`, `break`, `continue` | 구현 일부; 반복 제어 연결 | P0 |
| 함수·lambda·재귀·클로저 | `function NAME (...) {}`, `function {...} then ...`, `output(...)` | 구현 일부 | P0 |
| 배열·문자열·map/set/queue | 합의된 타입 이름과 `foreach`, 인덱스 0 기반 | dictionary, tuple, list·set·queue 기본 API(queue `peek/clear` 포함); 문자열 인덱싱과 `length()`는 UTF-8 코드포인트 기준 | P0 |
| 오류 처리 | `try/catch/finally`, `throw ErrorKind;` | throw와 try/catch/finally의 파서·컴파일러·VM 경로 구현; 런타임 검증은 미실행 | P0 |
| 모듈 | `import {...} from {...};`, `export {...};`, `like (...)` | built-in import만 지원 | P1 |
| 클래스·객체 | `class`, 공개 기본, `#`/`private`, `static`, `@` | 일부 런타임 존재, 표면 지원 불완전 | P1 |
| 타입 시스템 | 선언 타입, 검사·변환, alias, generic, union | 초기 런타임 타입 검사; 정적 analyzer·alias·generic 미구현 | P0/P1 |
| 표준 라이브러리 | `import {novi} from {novi};` | 출력·range 등 일부 내장; 모듈 registry 정리 필요 | P1 |
| 프로젝트 빌드·패키지 | `novi new`, 추후 `novi build/run/add` | scaffold만 구현 | P1 |
| 반복자·generator | `range` 지연 수열을 시작점으로 단계적 확장 | range 한정 | P2 |
| 비동기·동시성 | `async`, `await`; task/channel 표기 제안 필요 | 실행 모델 미구현 | P2 |
| 구조 분해·패턴 매칭 | 타입/컬렉션 모델 정리 후 Novi 표기 제안 | 미정 | P2 |
| 자원 정리·defer | `try/finally` 우선; 별도 구문은 제안 필요 | 미정 | P2 |
| 제너릭·인터페이스/trait | Novi 타입 체계 확정 후 설계 | 미정 | P2 |
| 메타프로그래밍·FFI | `@`, C ABI 중심의 제한된 확장 검토 | 미정 | P3 |
| reflection·unsafe | 보안·최적화 비용 검토 후 필요한 경우만 | 미정 | P3 |
| 디버거·프로파일러 | source span, stack trace, opcode/alloc 계측에서 확장 | 진단 기반만 있음 | P2 |

P0는 언어로 작은 프로그램을 작성하고 안전하게 실행하는 데 필요한 기능, P1은 여러 파일과 프로젝트로 확장하는 기능, P2는 대규모 앱/수치 프로그램의 표현력, P3는 목적이 확인된 저수준·동적 기능이다.

## 우선 구현 단계

### 단계 A — 언어 핵심 완결 (P0)

1. lexer/parser/AST/VM 간 지원 목록을 기능별로 일치시킨다.
2. 변수 scope, 초기화 전 읽기, 상수 binding, 타입 재대입 규칙을 의미 분석에서 검사한다.
3. 함수 인자/반환, 재귀, lambda capture와 호출 오류를 일관되게 만든다.
4. `break;`/`continue;`를 지원하고, 반복문 밖 사용은 파싱 오류로 거부한다.
5. list, tuple, dictionary, set, queue와 공통 iterable 프로토콜을 도입한다. `foreach (key, value in map)`을 포함한다.
6. 예외 처리를 AST→bytecode→VM까지 구현하고 stack trace와 `finally` 실행을 보장한다.
7. 문자열 escape, Unicode 정책, 보간 표기를 제안하고 문자열 라이브러리 최소 기능을 정한다.

### 단계 B — 모듈·타입·프로젝트 (P1)

1. import 경로 해석, 모듈 캐시, export 검증, 순환 import 오류를 구현한다.
2. resolver/semantic analyzer를 추가하여 이름, scope, 타입, 접근 제어 오류를 실행 전에 보고한다.
3. 정적 타입 annotation과 런타임 값 표현을 분리하고, 타입 alias 및 generic의 최소 형태를 설계한다.
4. class 생성자·상속·override·static/private 규칙을 하나의 의미 모델로 완성한다.
5. `novi.toml`을 읽고 `novi run`, `novi build`와 표준 프로젝트 layout을 제공한다.
6. 표준 라이브러리를 console, math, collections, text, filesystem처럼 작은 모듈로 분리한다. 외부 효과 모듈은 명시적 import를 요구한다.

### 단계 C — 반복자·수학·비동기 (P2)

1. range에서 일반 iterator/generator 프로토콜로 확장한다.
2. tensor와 숫자 타입의 연산 의미, 정밀도·shape 규칙을 우선 정한 뒤 수치 라이브러리를 단계별 제공한다.
3. `async`/`await`는 task/event loop, 오류 전달, 취소 규칙이 정해진 뒤 구현한다. 키워드만 파싱해 동시 실행처럼 보이게 두지 않는다.
4. 병렬 실행은 공유 메모리 잠금만으로 시작하지 않고 immutable data/message passing 후보를 비교한다.
5. 구조 분해와 패턴 매칭은 list/tuple/map/enum 모델과 함께 문법을 설계한다.

### 단계 D — 도구와 최적화 (P2/P3)

1. source map, Novi stack trace, formatter, LSP 진단을 마련한다.
2. opcode 수, 실행 시간, 할당량, 메모리를 계측하는 profiler를 제공한다.
3. 기준 프로그램을 고정하고 optimizer를 단계별 추가한다. 각 최적화는 성능 및 결과 일치 측정을 요구한다.
4. AOT/JIT는 VM baseline 이후 목표 성능과 배포 요구로 판단한다.
5. FFI는 C ABI의 명시적 타입 경계부터; unsafe와 reflection은 명확한 사용 사례가 있을 때만 설계한다.

## 문법 제안 원칙

기능 표기가 미정인 경우 다음 초안을 사용하되 사용자 승인 전에는 미확정으로 둔다.

```nvi
// 반복 종료/건너뛰기: 후보 키워드는 break, continue
break;
continue;

// 제네릭 타입: Novi의 기존 <> 표기 후보
list<integer>
dictionary<string, integer>

// 오류 payload: throw ErrorKind;를 확장하는 후보
throw InvalidValue('설명');

// 패턴 매칭: is 제어문과 구별되는 키워드 후보
match (value) {
    ...
}
```

이 초안은 코드 구현 명세가 아니다. 각 표기는 사용자가 정한 문장 끝 `;`, 중괄호 블록, 대소문자 무시 키워드, Novi 타입 표기와 충돌하는지 확인한 뒤 확정한다.

## 완료 기준

- 기능별 상태가 문서와 코드에서 일치한다.
- 합의된 Novi 문법만 구현되고 미결 문법은 임의로 고정되지 않는다.
- 구문·의미·런타임 오류에 Novi source span이 연결된다.
- 기능을 추가해도 VM 실행 엔진을 문법 표기에 직접 결합하지 않는다.
- “최적화” 주장은 대표 워크로드의 재현 가능한 측정과 함께 기록한다.
