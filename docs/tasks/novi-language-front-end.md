# Novi 프런트엔드 설계 작업장

> 이 문서는 Novi 소스 언어의 문법·키워드·기능·규칙을 사용자가 직접 정하는 작업장이다.
> 각 항목의 `Novi => ""` 안에 원하는 표기를 쓴다. 아직 정하지 않은 곳은 빈 문자열로 둔다.
> 다른 언어의 문법을 그대로 복사할 필요는 없다. 예시들은 선택지를 비교하기 위한 참고다.

백엔드의 AST, 바이트코드, VM, 객체 모델은 [백엔드 작업 문서](novi-object-oriented-backend.md)에 있다. 프런트엔드 표기를 결정한 뒤 파서와 의미 검사를 그 결정에 맞춘다.

## 작성 규칙

- `Novi => ""`만 사용자가 채운다. 다른 언어 예시는 참고용이다.
- 표기뿐 아니라 **규칙**도 정한다. 예: 재할당 가능 여부, 스코프, 우선순위, 오류 조건.
- 키워드 문법과 표준 라이브러리 함수를 구분한다. `print`가 예약어인지 보통 함수인지 정한다.
- 한 항목을 결정할 때 짧은 Novi 코드, 실행 결과, 잘못된 코드의 처리도 함께 적는다.
- 기능의 일부만 지원할 경우 지원 범위를 제한한다. 예: Java의 모든 annotation이나 C++의 모든 전처리 동작을 처음부터 구현할 필요는 없다.

## 1. 선언과 기본 표기

### 1.1 변수 선언

| 언어 | 예시 |
|---|---|
| Python | `name = value` |
| Java | `var name = value;` 또는 `Type name = value;` |
| Kotlin | `val name = value` (읽기 전용 참조), `var name = value` (재할당 가능) |
| C++ | `auto name = value;` 또는 `Type name = value;` |
| Go | `name := value` 또는 `var name Type` |
| JavaScript | `let name = value;` |
| TypeScript | `let name: Type = value;` |
| **Novi** | `=> ""` |

- 재선언 규칙: **Novi => ""**
- 선언과 초기화를 분리할 수 있는가: **Novi => ""**
- 초기값 생략 시 값 또는 오류: **Novi => ""**
- 재할당 가능한 변수와 읽기 전용 변수 표기: **Novi => ""**
- 변수 스코프(블록/함수/모듈/전역): **Novi => ""**
- 이름 가리기(shadowing) 허용 여부 및 처리: **Novi => ""**

### 1.2 상수 선언

| 언어 | 예시 |
|---|---|
| Python | 관례상 대문자 이름: `MAX_SIZE = 10` (언어 차원의 상수 아님) |
| Java | `final int limit = 10;` |
| Kotlin | `val limit = 10` |
| C++ | `const int limit = 10;`, `constexpr int limit = 10;` |
| Go | `const limit = 10` |
| JavaScript | `const limit = 10;` |
| TypeScript | `const limit: number = 10;` |
| **Novi** | `=> ""` |

- 상수의 재할당 금지 범위(바인딩만/객체 내부까지): **Novi => ""**
- 컴파일 시 계산하는 상수 지원 여부: **Novi => ""**

### 1.3 타입 선언과 타입 별칭

| 언어 | 예시 |
|---|---|
| Python | `age: int = 20`, `type UserId = int` (버전에 따른 타입 별칭 문법) |
| Java | `int age = 20;`, `class UserId { ... }` |
| Kotlin | `val age: Int = 20`, `typealias UserId = Int` |
| C++ | `int age = 20;`, `using UserId = int;`, `typedef int UserId;` |
| Go | `var age int = 20`, `type UserId int` |
| JavaScript | 런타임 타입 선언 없음 |
| TypeScript | `let age: number = 20;`, `type UserId = number;`, `interface User { ... }` |
| **Novi** | `=> ""` |

- Novi 타입 체계(정적/동적/점진적): **Novi => ""**
- 타입 추론, 타입 검사 시점, 타입 별칭: **Novi => ""**
- 기본 타입 이름과 의미: **Novi => ""**

### 1.4 문장, 블록, 주석, 이름

| 항목 | 참고 표기 | **Novi** |
|---|---|---|
| 문장 종료 | Python 줄바꿈 / Java·C++·Go·JS·TS `;` | `=> ""` |
| 블록 | Python 들여쓰기 / 나머지 대체로 `{ ... }` | `=> ""` |
| 한 줄 주석 | Python·Go·JS·TS `//` 또는 Python `#`; Java·C++ `//` | `=> ""` |
| 여러 줄 주석 | Python 보통 `'''...'''` 관례; C++·Java·Go·JS·TS `/* ... */` | `=> ""` |
| 문서 주석 | Python docstring / Java·C++·Go·JS 계열 문서 도구별 방식 | `=> ""` |
| 식별자 규칙 | 언어마다 Unicode·예약어·대소문자 규칙 상이 | `=> ""` |
| 키워드 및 예약어 | 언어마다 다름 | `=> ""` |
| 줄바꿈 연결/자동 세미콜론 | Python 괄호 연결 / JS 자동 세미콜론 삽입 등 | `=> ""` |

- 들여쓰기가 의미에 영향을 주는가: **Novi => ""**
- Unicode 식별자와 문자열 인코딩 규칙: **Novi => ""**
- 예약어를 변수명·속성명으로 쓸 수 있는 조건: **Novi => ""**

## 2. 리터럴, 연산자, 식 규칙

### 2.1 값 표기

| 값 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 정수 | `42` | `42` | `42` | `42` | `42` | `=> ""` |
| 소수 | `3.14` | `3.14` | `3.14` | `3.14` | `3.14` | `=> ""` |
| 문자열 | `'hi'`, `"hi"` | `"hi"` | `"hi"` | `"hi"` | `'hi'`, `"hi"`, `` `hi` `` | `=> ""` |
| 문자 | 길이 1 문자열 | `'a'` | `'a'` | `'a'` | 별도 문자 타입 없음 | `=> ""` |
| 참/거짓 | `True`, `False` | `true`, `false` | `true`, `false` | `true`, `false` | `true`, `false` | `=> ""` |
| null 값 | `None` | `null` | `nullptr` | `nil` (특정 타입의 nil) | `null`, `undefined` | `=> ""` |
| 배열/목록 | `[1, 2]` | `new int[]{1, 2}` / `List.of(1, 2)` | `std::array` / `std::vector` | `[]int{1, 2}` | `[1, 2]` | `=> ""` |
| 맵 | `{"a": 1}` | `Map.of("a", 1)` | `std::map` 초기화 | `map[string]int{"a": 1}` | `{"a": 1}` | `=> ""` |
| 집합 | `{1, 2}` | `Set.of(1, 2)` | `std::set` 초기화 | 표준 내장 집합 없음 | `new Set([1, 2])` | `=> ""` |
| 보간 문자열 | `f"Hi {name}"` | 문자열 연결/`String.formatted` 등 | 문자열 연결/`std::format` | `fmt.Sprintf` | `` `Hi ${name}` `` | `=> ""` |

- 정수 폭, 부호, 오버플로 처리: **Novi => ""**
- 부동소수점 종류와 정밀도: **Novi => ""**
- 정수 나눗셈, 나머지, 음수 나눗셈 규칙: **Novi => ""**
- 문자열 결합, 비교, 인덱싱, 길이 단위(바이트/코드포인트/문자소): **Novi => ""**
- 배열/맵/집합의 변경 가능성, 순서, 동등성: **Novi => ""**
- null과 초기화되지 않은 값의 구분: **Novi => ""**

### 2.2 연산자 표기와 우선순위

| 기능 | Python | Java / C++ / Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|
| 산술 | `+ - * / // % **` | `+ - * / %` (C++ 외 언어별 차이) | `+ - * / % **` | `=> ""` |
| 대입 | `=` | `=` | `=` | `=> ""` |
| 복합 대입 | `+=`, `-=`, ... | `+=`, `-=`, ... | `+=`, `-=`, ... | `=> ""` |
| 비교 | `== != < <= > >=` | 유사 | 유사 | `=> ""` |
| 논리 | `and or not` | `&& || !` | `&& || !` | `=> ""` |
| 비트 | `& | ^ ~ << >>` | 유사 | 유사 | `=> ""` |
| 증가/감소 | 없음 | `++ --` (Java/C++), Go는 별도 문장 | `++ --` | `=> ""` |
| 조건 선택 | `a if c else b` | `c ? a : b` | `c ? a : b` | `=> ""` |
| null 대체/안전 접근 | 관용 패턴 | 언어·버전별 제공 | `??`, `?.` | `=> ""` |
| 타입 검사/변환 | `isinstance`, 생성자 | cast/`instanceof` 등 | `typeof`, `instanceof` | `=> ""` |

- 전체 연산자 우선순위 표: **Novi => ""**
- 결합 방향 및 단항·대입 연산 규칙: **Novi => ""**
- `and`/`or` 계열의 단락 평가 여부: **Novi => ""**
- 객체의 `==`와 동일 객체 비교를 구분하는가: **Novi => ""**
- truthy/falsy 허용 여부와 조건식 타입: **Novi => ""**
- 암시적 형변환, 숫자/문자열 혼합 연산 처리: **Novi => ""**
- 연산자 오버로딩 허용 여부: **Novi => ""**

## 3. 함수와 호출

### 3.1 함수 선언

| 언어 | 예시 |
|---|---|
| Python | `def add(a, b): return a + b` |
| Java | `int add(int a, int b) { return a + b; }` |
| C++ | `int add(int a, int b) { return a + b; }` |
| Go | `func add(a int, b int) int { return a + b }` |
| JavaScript | `function add(a, b) { return a + b; }` |
| TypeScript | `function add(a: number, b: number): number { return a + b; }` |
| **Novi** | `=> ""` |

- 반환 타입 생략 시 규칙: **Novi => ""**
- 반환값 없는 함수의 표기와 결과: **Novi => ""**
- 재귀 호출 및 함수 이름의 스코프: **Novi => ""**

### 3.2 함수 기능

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 호출 | `f(x)` | `f(x)` | `f(x)` | `f(x)` | `f(x)` | `=> ""` |
| 기본 인수 | `def f(x=1)` | 오버로드 등 | 기본 인수값 | 직접 지원 안 함 | `function f(x=1)` | `=> ""` |
| 이름 있는 인수 | `f(x=1)` | 직접 지원 안 함 | 직접 지원 안 함 | 직접 지원 안 함 | 객체 인수 관용 | `=> ""` |
| 가변 인수 | `*args`, `**kwargs` | varargs `...` | parameter pack `...` | `...T` | `...args` | `=> ""` |
| 오버로딩 | 보통 단일 바인딩 | 시그니처 오버로딩 | 오버로딩 | 직접 지원 안 함 | 직접 지원 안 함 | `=> ""` |
| 익명 함수 | `lambda` | 람다 `->` | 람다 `[](...) {}` | 익명 `func(...) {}` | `(x) => x` | `=> ""` |
| 클로저 | 중첩 함수 | 람다 캡처 등 | 캡처 목록 | 함수 값/클로저 | lexical closure | `=> ""` |
| 다중 반환값 | 튜플 관용 | 객체/레코드 | `std::tuple` 등 | `return a, b` | 배열/객체 | `=> ""` |
| 생성기/지연 값 | `yield` | 스트림/반복자 | coroutine/범위 | `yield` (버전별) | `function*`, `yield` | `=> ""` |

- 값 전달/참조 전달 규칙: **Novi => ""**
- 기본 인수 평가 시점 및 가변 인수의 자료형: **Novi => ""**
- 오버로딩 선택 기준과 모호성 오류: **Novi => ""**
- 클로저가 캡처하는 변수와 수명: **Novi => ""**
- 함수가 일급 값인지, 중첩 정의 가능한지: **Novi => ""**
- `return` 생략 및 모든 코드 경로 반환 검사: **Novi => ""**

## 4. 제어 흐름과 오류 처리

### 4.1 분기와 반복

| 기능 | Python | Java / C++ / Go / JS / TS | **Novi** |
|---|---|---|---|
| 조건문 | `if cond:` / `elif` / `else` | `if (cond) { ... } else { ... }` (Python 외) | `=> ""` |
| 다중 분기 | `match` (버전별) | `switch`; Java/C++/Go/TS 지원 범위 다름 | `=> ""` |
| 조건식 | `a if cond else b` | `cond ? a : b` (Go에는 없음) | `=> ""` |
| while 반복 | `while cond:` | `while (cond) { ... }` | `=> ""` |
| 기본 for | `for x in items:` | C 스타일 `for(init; cond; step)` | `=> ""` |
| foreach | `for x in items:` | Java enhanced for / C++ range-for / Go `for range` / JS `for...of` | `=> ""` |
| 반복 종료 | `break`, `continue` | `break`, `continue` | `=> ""` |
| 반복 else | Python `for/while ... else` | 일반적으로 없음 | `=> ""` |
| 패턴 분해 | tuple unpacking / `match` | 구조적 패턴 매칭은 언어·버전별 | `=> ""` |

- 조건식은 반드시 Boolean이어야 하는가: **Novi => ""**
- switch/match의 패턴 종류, fall-through, 기본 분기: **Novi => ""**
- 반복 대상과 iterator 프로토콜: **Novi => ""**
- 라벨 반복과 중첩 반복에서의 `break`/`continue`: **Novi => ""**
- 패턴 매칭의 exhaustiveness 검사: **Novi => ""**

### 4.2 오류 처리

| 기능 | Python | Java / C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|
| 예외 발생 | `raise Error(...)` | `throw new E(...)` / `throw E{...}` | 관례적으로 error 반환; `panic`도 있음 | `throw new Error(...)` | `=> ""` |
| 예외 처리 | `try/except/finally` | `try/catch/finally` (Go 없음) | `if err != nil` | `try/catch/finally` | `=> ""` |
| 오류 반환값 | 예외와 반환 모두 | 예외 중심 | `(value, error)` 관례 | 예외/Promise reject | `=> ""` |
| 오류 전파 | 예외가 상위로 전달 | 예외가 상위로 전달 | 명시적 반환 | 예외/Promise reject | `=> ""` |
| 자원 정리 | `with` / `finally` | try-with-resources / RAII | `defer` | `finally` / `using` (버전별) | `=> ""` |

- 오류 타입 계층과 사용자 정의 오류: **Novi => ""**
- 복구 가능한 오류와 프로그래밍 오류 구분: **Novi => ""**
- 오류 전파 단축 문법, stack trace 표시: **Novi => ""**
- 정리 코드 실행 보장 시점: **Novi => ""**

## 5. 객체, 클래스, 복합 타입

### 5.1 클래스와 인스턴스

| 언어 | 선언·생성 예시 |
|---|---|
| Python | `class Dog: ...`; `dog = Dog()` |
| Java | `class Dog { ... }`; `Dog dog = new Dog();` |
| C++ | `class Dog { ... };`; `Dog dog;` 또는 `auto dog = Dog();` |
| Go | `type Dog struct { ... }`; `dog := Dog{}` |
| JavaScript | `class Dog { ... }`; `const dog = new Dog();` |
| TypeScript | `class Dog { ... }`; `const dog = new Dog();` |
| **Novi** | `=> ""` |

### 5.2 클래스 기능별 표기

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 생성자 | `__init__(self)` | 클래스명 메서드 | 클래스명 메서드 | 별도 생성자 없음/함수 관례 | `constructor()` | `=> ""` |
| 현재 인스턴스 | `self` | `this` | `this` | receiver 매개변수 | `this` | `=> ""` |
| 필드 접근 | `obj.name` | `obj.name` | `obj.name` | `obj.Name` | `obj.name` | `=> ""` |
| 상속 | `class B(A):` | `class B extends A` | `class B : public A` | 내장(embedding) | `class B extends A` | `=> ""` |
| 인터페이스 계약 | Protocol/ABC 등 | `implements I` | 추상 클래스/개념 | interface 암시 구현 | `implements I` | `=> ""` |
| 접근 제어 | 관례/이름 맹글링 | `public/private/protected` | 접근 지정자 | export 대문자 규칙 | `public/private/protected` | `=> ""` |
| 정적 멤버 | `@staticmethod`/`@classmethod` | `static` | `static` | 타입 함수 관례 | `static` | `=> ""` |
| 프로퍼티 | `@property` | getter/setter | 메서드 관례 | 메서드 관례 | getter/setter | `=> ""` |
| 오버라이드 표시 | 관례/데코레이터 | `@Override` | `override` | 메서드 재정의 대신 embedding | 재정의/데코레이터는 실험적 | `=> ""` |

- 클래스가 타입이자 값인지, 클래스 자체의 메서드 호출 규칙: **Novi => ""**
- 생성자 상속·부모 생성자 호출·초기화 순서: **Novi => ""**
- 필드 초기값 및 초기화 블록 실행 순서: **Novi => ""**
- 상속 방식(단일/다중/인터페이스/trait) 및 다이아몬드 처리: **Novi => ""**
- 메서드 디스패치(정적/동적) 및 오버라이드 검증: **Novi => ""**
- 접근 제어의 컴파일 타임/런타임 보장 여부: **Novi => ""**
- 추상 클래스·인터페이스·trait·extension 지원 여부: **Novi => ""**
- 구조체/record/data class/enum/union 지원 여부: **Novi => ""**
- 프로퍼티, 연산자 오버로딩, 사용자 정의 변환: **Novi => ""**

### 5.3 제네릭과 타입 기능

| 기능 | Python | Java | C++ | Go | TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 제네릭 | `list[int]`, `TypeVar` | `List<T>` | `template<class T>` | `List[T]` | `Array<T>` | `=> ""` |
| 타입 제한 | Protocol/TypeVar bound | `T extends Base` | concepts/요건 | interface constraint | `T extends U` | `=> ""` |
| 합집합 타입 | `int | str` (버전별) | sealed hierarchy 등 | `std::variant` | interface/type set | `A \| B` | `=> ""` |
| 선택값 | `Optional[T]` / `T \| None` | `Optional<T>` | `std::optional<T>` | pointer/두 값 등 | `T \| undefined` | `=> ""` |
| 열거형 | `enum.Enum` | `enum` | `enum class` | `iota` 관례 | `enum` | `=> ""` |
| reflection | `inspect`, 런타임 정보 | Reflection API | RTTI/메타프로그래밍 | `reflect` | 런타임 reflection 제한적 | `=> ""` |

- 제네릭이 컴파일 시 지워지는지/런타임에 보존되는지: **Novi => ""**
- 공변/반공변, 타입 추론, 타입 경계 규칙: **Novi => ""**
- nullable 타입 및 exhaustive enum 처리: **Novi => ""**

## 6. 모듈, 패키지, 코드 생성

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 코드 가져오기 | `import x`, `from x import y` | `import p.Type;` / module | `#include <...>` / C++20 `import` | `import "module/path"` | `import { x } from "m"` | `=> ""` |
| 공개 내보내기 | 기본 공개 | `public` | 선언/헤더 공개 관례 | 대문자 이름 | `export` | `=> ""` |
| 별칭 가져오기 | `import x as y` | `import p.Type` | namespace alias | import alias | `import {x as y}` | `=> ""` |
| 의존성 관리 | pip/uv/pyproject | Maven/Gradle | 패키지 관리자별 | Go modules | npm/pnpm/yarn | `=> ""` |
| 매크로/전처리 | 없음(외부 도구) | annotation processor | `#define`, `#if`, 템플릿 | `go generate`/build tags | 빌드 도구 변환 | `=> ""` |
| annotation/decorator | decorator | annotation | attribute | 없음/주석 도구 | decorator | `=> ""` |
| FFI/native 연결 | C API/확장 | JNI/Foreign Function API | C ABI/직접 연결 | cgo | Node addon/WebAssembly 등 | `=> ""` |

- 순환 import, 모듈 초기화 1회 여부, 검색 경로: **Novi => ""**
- package/module namespace와 visibility: **Novi => ""**
- 라이브러리 버전 고정·lockfile·빌드 재현성: **Novi => ""**
- 매크로가 토큰/AST/코드 중 무엇을 변환하는가: **Novi => ""**
- compile-time reflection 또는 code generation: **Novi => ""**

### 6.1 전처리 지시문 참고

| 언어 | 대표 표기 | 범위 | **Novi** |
|---|---|---|---|
| C++ | `#include`, `#define`, `#if`, `#ifdef`, `#pragma` | 전처리기가 토큰/조건부 소스를 처리 | `=> ""` |
| Java | `@Annotation` | 컴파일러/도구가 annotation을 읽고 처리 | `=> ""` |
| Python | decorator `@decorator` | 함수/클래스 생성 결과를 감싸거나 변환 | `=> ""` |
| Go | build constraints `//go:build`, `//go:generate` | 빌드 선택/외부 생성 명령 | `=> ""` |
| TypeScript | `/// <reference ... />`, compiler options | 타입 검사·모듈/빌드 동작에 지시 | `=> ""` |
| JavaScript | 표준 전처리 지시문 없음 | 보통 bundler/transpiler가 처리 | `=> ""` |

- Novi에 전처리 단계가 필요한가, 문법 자체로 해결할 것인가: **Novi => ""**
- 매크로를 제공한다면 실행 가능 범위와 보안 경계: **Novi => ""**

## 7. 동시성, 비동기, 메모리

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 비동기 선언 | `async def` | `CompletionStage`/가상 스레드 등 | coroutine 라이브러리/표준 기능 | goroutine 호출 | `async function` | `=> ""` |
| 대기 | `await` | `.get()` 등 | `co_await` | 채널/동기화 | `await` | `=> ""` |
| 병렬 단위 | thread/process/asyncio task | thread/virtual thread | `std::thread`/task | goroutine | worker/thread/event loop | `=> ""` |
| 메시지 전달 | queue/asyncio | queue/channel 라이브러리 | queue 라이브러리 | channel | Promise/message channel | `=> ""` |
| 메모리 관리 | 참조 카운트+GC | GC | 수동/RAII/smart pointer | GC | GC | `=> ""` |
| 자원 정리 | context manager | try-with-resources | RAII/destructor | `defer` | `using`/`finally` 등 | `=> ""` |

- 실행 모델(단일 스레드/스레드/actor/task): **Novi => ""**
- 공유 메모리·불변 데이터·메시지 전달 정책: **Novi => ""**
- 취소, timeout, 오류 전파와 structured concurrency: **Novi => ""**
- GC/참조 카운트/소유권/수동 관리 및 결정적 해제: **Novi => ""**

## 8. 기본 기능과 입출력

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 출력 | `print(x)` | `System.out.println(x)` | `std::cout << x` / `std::print` | `fmt.Println(x)` | `console.log(x)` | `=> ""` |
| 입력 | `input()` | `Scanner`/`Console` | `std::cin` | `fmt.Scan`/`bufio` | 환경 API별 입력 | `=> ""` |
| 길이 | `len(x)` | `x.length()`/`length` | `std::size(x)` | `len(x)` | `x.length` / `Map.size` | `=> ""` |
| 문자열 형식화 | f-string/`format` | `Formatter` | `std::format`/stream | `fmt.Sprintf` | template literal | `=> ""` |
| 시간 | `datetime` | `java.time` | `<chrono>` | `time` | `Date`/`Temporal` | `=> ""` |
| 난수 | `random`/`secrets` | `java.util.Random`/`SecureRandom` | `<random>` | `math/rand`, `crypto/rand` | `Math.random`/crypto API | `=> ""` |

- 출력은 예약 구문인가, 수정 가능한 표준 함수인가: **Novi => ""**
- 기본 내장 이름, shadowing 가능 여부: **Novi => ""**
- 표준 입력·출력 스트림과 encoding: **Novi => ""**
- sandbox에서 파일·환경변수·프로세스·네트워크를 허용하는 범위: **Novi => ""**

## 9. 키워드와 내장 이름 비교표

아래는 대표 문법 요소다. 각 언어의 모든 예약어와 내장 이름 목록은 이 문서 끝의 공식/참고 문서에서 확인한다. 같은 단어라도 키워드인지 표준 라이브러리 이름인지는 서로 다를 수 있다.

| 역할 | Python | Java | C++ | Go | JavaScript | TypeScript | **Novi** |
|---|---|---|---|---|---|---|---|
| 변수/상수 | 대입 / 대문자 관례 | 타입 / `var`, `final` | 타입 / `auto`, `const`, `constexpr` | `:=`, `var`, `const` | `let`, `const`, `var` | JS + 타입 표기 | `=> ""` |
| 함수 | `def`, `lambda` | 반환 타입·메서드, 람다 `->` | 반환 타입·람다 | `func` | `function`, `=>` | JS + 타입 표기 | `=> ""` |
| 타입/클래스 | `class`, `type` | `class`, `interface`, `enum`, `record` | `class`, `struct`, `enum`, `template` | `type`, `struct`, `interface` | `class` | JS + `type`, `interface`, `enum` | `=> ""` |
| 분기/반복 | `if`, `match`, `for`, `while` | `if`, `switch`, `for`, `while` | `if`, `switch`, `for`, `while` | `if`, `switch`, `for`, `range` | `if`, `switch`, `for`, `while` | JS + 타입 narrowing | `=> ""` |
| 오류/자원 | `raise`, `try`, `except`, `with` | `throw`, `try`, `catch`, `finally` | `throw`, `try`, `catch`, RAII | `error`, `defer`, `panic`, `recover` | `throw`, `try`, `catch`, `finally` | JS + 타입 검사 | `=> ""` |
| 모듈 | `import`, `from`, `as` | `package`, `import`, `module` | `#include`, `import`, `module` | `package`, `import` | `import`, `export` | JS + type-only import/export | `=> ""` |
| 매크로/메타 | decorator | annotation | `#define`, template, `constexpr` | compiler directive comments | 표준에는 없음 | triple-slash directive/type operator | `=> ""` |

## 10. 표준 라이브러리와 생태계 조사표

표준 라이브러리의 모든 함수·클래스·상수는 언어 버전마다 바뀌고 항목 수가 매우 많다. 이 표는 각 언어의 공식 전체 색인과 주요 영역을 제공한다. Novi 사용자는 가져올 **기능 영역**을 정하고, 이름·호출 형식은 `Novi => ""`에 정한다. 3rd-party 패키지 전체는 언어 표준 라이브러리와 별도의 생태계다.

### 10.1 Python

- 공식 문법·키워드: [Language Reference](https://docs.python.org/3/reference/), [Keywords](https://docs.python.org/3/reference/lexical_analysis.html#keywords)
- 표준 라이브러리 전체 모듈 색인: [The Python Standard Library](https://docs.python.org/3/library/index.html)
- built-in 함수·상수·예외 전체 색인: [Built-in Functions](https://docs.python.org/3/library/functions.html)
- 3rd-party 배포 패키지 색인: [Python Package Index (PyPI)](https://pypi.org/)

| 표준 라이브러리 영역 | 대표 모듈 예시 | Novi 채택/표기 |
|---|---|---|
| 텍스트·문자 인코딩·정규식 | `string`, `textwrap`, `unicodedata`, `re`, `codecs` | `=> ""` |
| 숫자·수학·난수 | `math`, `cmath`, `decimal`, `fractions`, `random`, `statistics` | `=> ""` |
| 날짜·시간·지역화 | `datetime`, `zoneinfo`, `calendar`, `locale` | `=> ""` |
| 자료구조·반복·함수 도구 | `collections`, `array`, `heapq`, `bisect`, `itertools`, `functools`, `operator` | `=> ""` |
| 파일·경로·임시 파일 | `pathlib`, `os`, `io`, `glob`, `tempfile`, `shutil`, `fileinput` | `=> ""` |
| 직렬화·데이터 형식 | `json`, `csv`, `pickle`, `struct`, `configparser`, `tomllib`, `xml` | `=> ""` |
| 압축·아카이브 | `zlib`, `gzip`, `bz2`, `lzma`, `zipfile`, `tarfile` | `=> ""` |
| 암호·해시 | `hashlib`, `hmac`, `secrets` | `=> ""` |
| 프로세스·명령행·환경 | `sys`, `argparse`, `getopt`, `subprocess`, `signal`, `atexit`, `logging` | `=> ""` |
| 동시성·비동기 | `threading`, `multiprocessing`, `concurrent.futures`, `asyncio`, `queue` | `=> ""` |
| 네트워크·인터넷 | `socket`, `ssl`, `urllib`, `http`, `ftplib`, `smtplib`, `email`, `webbrowser` | `=> ""` |
| 데이터베이스 | `sqlite3`, `dbm` | `=> ""` |
| 개발·디버깅·검사 | `pdb`, `traceback`, `inspect`, `ast`, `dis`, `timeit`, `doctest`, `unittest` | `=> ""` |
| 운영체제·플랫폼 | `platform`, `ctypes`, `mmap`, `resource`, `shlex` | `=> ""` |

- Novi 기본 제공 Python 유사 기능 영역: **Novi => ""**
- 설치형 패키지와 표준 모듈의 구분·가져오기 표기: **Novi => ""**

### 10.2 Java

- 공식 문법: [Java Language Specification](https://docs.oracle.com/javase/specs/)
- Java SE 전체 API 패키지·클래스·메서드 색인: [Java SE API Specification](https://docs.oracle.com/en/java/javase/25/docs/api/index.html)
- 언어 키워드 참고: [Java Language Keywords](https://docs.oracle.com/javase/tutorial/java/nutsandbolts/_keywords.html)
- 생태계/외부 패키지: Maven Central, Gradle Plugin Portal 등(표준 라이브러리와 별도)

| Java SE API 영역/대표 패키지 | 용도 | Novi 채택/표기 |
|---|---|---|
| `java.lang` | 핵심 타입, 문자열, 수학, 예외, process/runtime | `=> ""` |
| `java.util`, `java.util.function` | 컬렉션, 날짜 보조, 난수, 함수형 인터페이스 | `=> ""` |
| `java.util.stream` | 컬렉션 파이프라인 | `=> ""` |
| `java.io`, `java.nio`, `java.nio.file` | stream, buffer, 파일 시스템 | `=> ""` |
| `java.net`, `java.net.http` | 소켓, URI, HTTP client | `=> ""` |
| `java.time` | 날짜·시간 API | `=> ""` |
| `java.text`, `java.util.regex` | 포맷, locale, 정규식 | `=> ""` |
| `java.math` | 큰 정수·정밀 소수 | `=> ""` |
| `java.util.concurrent` 및 하위 패키지 | thread pool, 동기화, concurrent collection | `=> ""` |
| `java.util.concurrent.Flow` | reactive-streams 인터페이스 | `=> ""` |
| `java.sql`, `javax.sql` | JDBC 데이터베이스 연결 | `=> ""` |
| `java.security`, `javax.crypto` | 권한·키·암호 기능 | `=> ""` |
| `java.lang.reflect`, `java.lang.invoke` | reflection 및 동적 호출 | `=> ""` |
| `java.xml` 계열 | XML 처리 | `=> ""` |
| `java.desktop` 계열 | AWT/Swing/이미지·데스크톱 | `=> ""` |
| `java.compiler`, `javax.tools` | 컴파일러 도구 API | `=> ""` |

- Novi 표준 라이브러리에서 필요한 Java API 영역: **Novi => ""**
- Java class 이름/API를 호환할지, Novi식 API로 다시 설계할지: **Novi => ""**

### 10.2a Kotlin

- 공식 문법·언어 기능: [Kotlin Language Documentation](https://kotlinlang.org/docs/kotlin-language-features-and-proposals.html)
- 공식 표준 라이브러리 API 전체 색인: [Kotlin Standard Library](https://kotlinlang.org/api/core/kotlin-stdlib/)
- 공식 Coroutines 라이브러리 문서: [kotlinx.coroutines](https://kotlinlang.org/api/kotlinx.coroutines/)
- 패키지/의존성은 주로 Gradle 또는 Maven으로 관리하며, Kotlin 표준 라이브러리와 별도다.

| Kotlin 기능/표기 | 예시 | Novi 표기/규칙 |
|---|---|---|
| 읽기 전용/가변 바인딩 | `val x = 1`; `var y = 2` | `=> ""` |
| 타입 추론/타입 지정 | `val x: Int = 1` | `=> ""` |
| 함수/한 줄 함수 | `fun add(a: Int, b: Int) = a + b` | `=> ""` |
| 문자열 보간 | `"Hello, $name"`, `"${f()}"` | `=> ""` |
| nullable 타입/안전 호출 | `String?`, `user?.name`, `?:`, `!!` | `=> ""` |
| 클래스/생성자 | `class User(val name: String)` | `=> ""` |
| data/sealed/enum class | `data class`, `sealed class`, `enum class` | `=> ""` |
| 상속/구현 | `open class B : A()`, `class C : I` | `=> ""` |
| object/companion object | `object Registry`, `companion object` | `=> ""` |
| 접근 제어 | `public`, `private`, `protected`, `internal` | `=> ""` |
| 확장 함수/프로퍼티 | `fun String.wordCount() = ...` | `=> ""` |
| 람다/고차 함수 | `{ x -> x * 2 }`, trailing lambda | `=> ""` |
| 제네릭/variance | `fun <T> copy(x: T): T`; `in`/`out` | `=> ""` |
| 컬렉션 연산 | `map`, `filter`, `fold`, `sequence` | `=> ""` |
| 구조 분해/패턴 | `val (a, b) = pair`; `when` | `=> ""` |
| 예외/자원 처리 | `try/catch/finally`; `use { ... }` | `=> ""` |
| coroutine | `suspend fun`, `launch`, `async`, `await` (라이브러리) | `=> ""` |
| annotation | `@Annotation` | `=> ""` |

| Kotlin 표준 라이브러리 영역 | 대표 API/패키지 | Novi 채택/표기 |
|---|---|---|
| 핵심 타입·문자열·수학 | `kotlin`, `kotlin.text`, `kotlin.math` | `=> ""` |
| 컬렉션·시퀀스·반복 | `kotlin.collections`, `kotlin.sequences`, `kotlin.ranges` | `=> ""` |
| 함수형·결과 타입 | `kotlin.Function`, `kotlin.Result`, `kotlin.comparisons` | `=> ""` |
| I/O·파일 | `kotlin.io`, `kotlin.io.path` (플랫폼 지원 범위별) | `=> ""` |
| 시간 | `kotlin.time` | `=> ""` |
| reflection·annotation | `kotlin.reflect`, `kotlin.annotation` (별도 런타임/의존성 조건 가능) | `=> ""` |
| 플랫폼 API | Kotlin/JVM, Kotlin/Native, Kotlin/JS 등에 따라 상이 | `=> ""` |
| Coroutines 확장 | `kotlinx.coroutines` (표준 라이브러리 아님) | `=> ""` |

- Kotlin식 null 안전성, 확장 함수, data/sealed class에서 Novi에 필요한 기능: **Novi => ""**
- Kotlin Multiplatform처럼 여러 실행 환경을 목표로 할지: **Novi => ""**

### 10.3 C++

- 공식 표준 초안·언어 규칙: [C++ Working Draft](https://eel.is/c++draft/)
- 헤더·클래스·함수 색인(검색하기 쉬운 참고): [C++ Standard Library](https://en.cppreference.com/w/cpp/standard_library)
- 전처리 지시문 참고: [C++ Preprocessor](https://en.cppreference.com/w/cpp/preprocessor)
- 생태계: Conan, vcpkg, CMake FetchContent 등(표준 라이브러리와 별도)

| C++ 표준 라이브러리 영역/헤더 | 용도 | Novi 채택/표기 |
|---|---|---|
| 언어 지원: `<type_traits>`, `<concepts>`, `<limits>`, `<version>` | 컴파일 시 타입 특성·제약 | `=> ""` |
| 진단·예외: `<cassert>`, `<exception>`, `<stdexcept>`, `<system_error>` | 오류 및 assertion | `=> ""` |
| 메모리: `<memory>`, `<new>`, `<scoped_allocator>` | allocator, smart pointer | `=> ""` |
| 유틸리티: `<utility>`, `<tuple>`, `<optional>`, `<variant>`, `<expected>` | pair, 이동, 선택/결과 타입 | `=> ""` |
| 컨테이너: `<array>`, `<vector>`, `<deque>`, `<list>`, `<map>`, `<unordered_map>`, `<set>`, `<queue>`, `<stack>` | 기본 자료구조 | `=> ""` |
| 반복자·범위·알고리즘: `<iterator>`, `<ranges>`, `<algorithm>`, `<numeric>` | 탐색·정렬·변환·누적 | `=> ""` |
| 문자열·텍스트: `<string>`, `<string_view>`, `<format>`, `<regex>`, `<locale>` | 문자열·형식화·정규식 | `=> ""` |
| 수학·난수: `<cmath>`, `<complex>`, `<random>`, `<numbers>` | 수학 및 난수 생성 | `=> ""` |
| 시간: `<chrono>`, `<ctime>` | 시간점·기간·달력 | `=> ""` |
| 입출력: `<iostream>`, `<fstream>`, `<sstream>`, `<filesystem>`, `<print>` | console, 파일, 경로 | `=> ""` |
| 동시성: `<thread>`, `<mutex>`, `<atomic>`, `<future>`, `<condition_variable>`, `<semaphore>`, `<latch>` | thread·동기화 | `=> ""` |
| 날짜/비트/저수준: `<bit>`, `<bitset>`, `<cstddef>`, `<cstdint>`, `<cstring>` | 메모리·비트·정수 폭 | `=> ""` |
| C 호환 헤더 | `<cstdio>`, `<cstdlib>`, `<cmath>` 등 | C API 호환 | `=> ""` |

- Novi에서 저수준 메모리·포인터·수명 제어를 노출할지: **Novi => ""**
- header 기반 import인지 패키지/모듈 기반인지: **Novi => ""**
- 컴파일 시점 계산·템플릿 수준 메타프로그래밍을 노출할지: **Novi => ""**

### 10.4 Go

- 공식 언어 명세: [The Go Programming Language Specification](https://go.dev/ref/spec)
- 공식 문서와 패키지 안내: [Go Documentation](https://go.dev/doc/)
- 표준 패키지 전체 색인: [Go Standard Library](https://pkg.go.dev/std)
- 내장 함수·미리 선언 식별자: [builtin package](https://pkg.go.dev/builtin)
- 외부 모듈 색인: [pkg.go.dev](https://pkg.go.dev/)

| Go 표준 패키지 영역/대표 패키지 | 용도 | Novi 채택/표기 |
|---|---|---|
| 기본 입출력·문자열: `fmt`, `io`, `bufio`, `strings`, `strconv`, `bytes` | 포맷·stream·텍스트 | `=> ""` |
| 자료구조·정렬: `slices`, `maps`, `cmp`, `sort`, `container/*` | slice/map 도구·정렬·heap/list/ring | `=> ""` |
| 수학·난수: `math`, `math/big`, `math/rand/v2`, `crypto/rand` | 수학·큰 수·난수 | `=> ""` |
| 날짜·실행시간: `time`, `runtime`, `runtime/trace` | 시간·runtime 정보 | `=> ""` |
| 파일·OS: `os`, `io/fs`, `path`, `path/filepath`, `embed`, `syscall` | 파일과 OS 접근 | `=> ""` |
| encoding·compression: `encoding/*`, `compress/*`, `archive/*` | JSON·텍스트·바이너리·압축 | `=> ""` |
| 네트워크·프로토콜: `net`, `net/http`, `net/url`, `mime`, `textproto` | socket·HTTP·URI | `=> ""` |
| 동시성·context: `context`, `sync`, `sync/atomic` | 취소·lock·atomic | `=> ""` |
| 암호·인증: `crypto/*`, `hash`, `vendor/golang.org/x/crypto` 일부는 별도 | 해시·서명·암호 | `=> ""` |
| 데이터베이스: `database/sql`, `database/sql/driver` | DB 공통 인터페이스 | `=> ""` |
| 테스트·진단: `testing`, `log`, `slog`, `runtime/pprof`, `debug/*` | 테스트·로그·프로파일 | `=> ""` |
| parser/compiler 도구: `go/ast`, `go/parser`, `go/token`, `go/types`, `go/format` | Go 소스 분석/format | `=> ""` |

- Go식 interface 암시 구현·error 반환·`defer`를 Novi에서 제공할지: **Novi => ""**
- goroutine/channel에 대응하는 Novi 동시성 모델: **Novi => ""**

### 10.5 JavaScript

- ECMAScript 공식 표준: [ECMAScript Language Specification](https://tc39.es/ecma262/)
- 문법·연산자·statement·built-in 참고 색인: [MDN JavaScript Reference](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference)
- built-in 객체 색인: [MDN Standard built-in objects](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects)
- JavaScript 표준은 파일 시스템·HTTP 서버 같은 OS API를 포함하지 않는다. 브라우저와 Node.js API는 실행 환경 라이브러리다.
- 패키지 생태계: [npm](https://www.npmjs.com/)

| ECMAScript 내장 영역/대표 객체 | 용도 | Novi 채택/표기 |
|---|---|---|
| 기본 값·전역: `Object`, `Function`, `Boolean`, `Symbol`, `globalThis` | 기본 객체/전역 환경 | `=> ""` |
| 숫자·수학: `Number`, `BigInt`, `Math` | 수 표현 및 연산 | `=> ""` |
| 텍스트·정규식: `String`, `RegExp` | 문자열과 패턴 검색 | `=> ""` |
| 컬렉션: `Array`, `Map`, `Set`, `WeakMap`, `WeakSet`, `Iterator` | 순서형·키값·집합·반복 | `=> ""` |
| 날짜·국제화: `Date`, `Intl` | 시간·locale 포맷 | `=> ""` |
| 비동기: `Promise`, `AsyncFunction`, `Atomics`, `SharedArrayBuffer` | Promise 및 공유 메모리 연산 | `=> ""` |
| 오류: `Error` 및 하위 오류 타입 | 오류 값 | `=> ""` |
| JSON·URI: `JSON`, `URL` 일부 환경별, `encodeURI` 계열 | 직렬화와 URI 도구 | `=> ""` |
| 메타·reflection: `Reflect`, `Proxy` | 동적 객체 관찰/중재 | `=> ""` |

- Novi의 `undefined`에 대응하는 값이 별도 필요한가: **Novi => ""**
- prototype 기반 동작을 노출할지, class 기반 모델로 제한할지: **Novi => ""**
- 브라우저 DOM/Fetch/Storage 같은 환경 API를 표준 범위로 볼지: **Novi => ""**

### 10.6 TypeScript

- 공식 Handbook·reference 전체 색인: [TypeScript Documentation](https://www.typescriptlang.org/docs/)
- 타입 관련 기본 정의: TypeScript는 JavaScript 위에 정적 타입 검사와 타입 문법을 더하며, 타입 표기는 보통 실행 결과에 남지 않는다.
- 표준 API 타입 정의는 TypeScript compiler의 `lib.*.d.ts`에서 제공되고, 실제 런타임 기능은 JavaScript/host 환경에 달려 있다.
- 외부 패키지: npm 생태계의 JavaScript 패키지와 TypeScript 선언 패키지

| TypeScript 기능 영역 | 대표 기능 | Novi 채택/표기 |
|---|---|---|
| 변수·함수 타입 | annotation, inference, contextual typing | `=> ""` |
| 객체 타입 | property, optional property, readonly, index signature | `=> ""` |
| 합집합·교차 | `A \| B`, `A & B`, discriminated union | `=> ""` |
| 타입 좁히기 | `typeof`, `in`, `instanceof`, 사용자 type guard | `=> ""` |
| 제네릭 | type parameter, constraint, inference | `=> ""` |
| 타입 연산자 | `keyof`, `typeof`, indexed access, conditional type | `=> ""` |
| mapped/template literal types | 타입 생성·변환 | `=> ""` |
| utility types | `Partial`, `Required`, `Readonly`, `Pick`, `Omit`, `Record`, `Exclude`, `Extract`, `ReturnType`, `Parameters`, `Awaited`, `NoInfer` 등 | `=> ""` |
| class/interface/enum | 선언 병합·implements·abstract·decorator 등 | `=> ""` |
| module/type import | ESM, type-only import/export, declaration file | `=> ""` |
| 컴파일러 설정 | `tsconfig`, target, module, strictness, JSX | `=> ""` |

- 정적 타입 기능을 Novi 컴파일러에서 어느 수준까지 구현할지: **Novi => ""**
- 타입 제거 후 JavaScript 생성, Novi VM 바이트코드 생성, 두 경로 모두 중 무엇을 목표로 할지: **Novi => ""**
- structural/nominal typing, `any`/`unknown`/union/intersection 유사 기능: **Novi => ""**

## 11. 라이브러리 선택 양식

각 라이브러리 영역에서 Novi에 넣을 때 아래 양식을 복사해 구체화한다. 함수 하나하나를 무조건 복제하는 대신 사용 사례·호환성·실행 환경을 정한다.

```markdown
### 라이브러리/기능 이름
- 참고 언어와 공식 API:
- Novi 표기: => ""
- 목적과 대표 사용 예:
- 입력/반환 타입:
- 오류 처리:
- 동기/비동기:
- 권한(파일·네트워크·프로세스 등):
- 첫 버전에 포함: => ""
- 호환성 목표 또는 Novi식 재설계: => ""
```

### 기능 영역 전체 선택

- 문자열·Unicode·정규식: **Novi => ""**
- 수학·큰 수·난수·통계: **Novi => ""**
- 배열·맵·집합·정렬·반복자: **Novi => ""**
- 날짜·시간·시간대·locale: **Novi => ""**
- 파일·경로·환경변수·프로세스: **Novi => ""**
- JSON·CSV·바이너리·압축: **Novi => ""**
- 네트워크·HTTP·TLS·URL: **Novi => ""**
- 데이터베이스: **Novi => ""**
- 암호·해시·인증: **Novi => ""**
- 동시성·비동기·취소: **Novi => ""**
- 로그·진단·성능 측정: **Novi => ""**
- 테스트·assertion·mock: **Novi => ""**
- compiler/parser/AST reflection: **Novi => ""**
- 그래픽·이미지·오디오·UI: **Novi => ""**
- 운영체제·플랫폼 API: **Novi => ""**

## 12. 구현 우선순위 결정

기능을 선택할 때는 아래 우선순위와 비고를 기록한다.

| 기능 | 단계(P0/P1/P2/제외) | Novi 표기 결정 | 필요한 백엔드 변경 | 메모 |
|---|---|---|---|---|
| 값·변수·연산·출력 |  | `=> ""` |  |  |
| 함수·스코프·클로저 |  | `=> ""` |  |  |
| 배열·맵·반복문 |  | `=> ""` |  |  |
| 클래스·필드·메서드 |  | `=> ""` |  |  |
| 상속·인터페이스·제네릭 |  | `=> ""` |  |  |
| 오류 처리·자원 관리 |  | `=> ""` |  |  |
| 모듈·패키지 |  | `=> ""` |  |  |
| 파일·네트워크·DB |  | `=> ""` |  |  |
| async·동시성 |  | `=> ""` |  |  |
| 매크로·annotation·codegen |  | `=> ""` |  |  |

## 다음 결정 순서

1. 선언, 문장 종료, 블록, 주석
2. 기본 값, 타입 체계, 연산자 우선순위
3. 함수·스코프·반환·오류 처리
4. 객체·클래스·생성자·접근 제어·상속
5. 컬렉션·반복 프로토콜·표준 함수
6. 모듈·패키지·라이브러리 정책
7. 메모리·동시성·매크로·FFI

각 결정을 내리면 Novi 예시 코드를 추가하고, 백엔드 task 문서와 AST/바이트코드 설계를 함께 갱신한다.
