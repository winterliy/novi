# Novi 프런트엔드 설계 작업장

> 이 문서는 Novi 소스 언어의 문법·키워드·기능·규칙을 사용자가 직접 정하는 작업장이다.
> 각 항목의 `Novi => ""` 안에 원하는 표기를 쓴다. 아직 정하지 않은 곳은 빈 문자열로 둔다.
> 다른 언어의 문법을 그대로 복사할 필요는 없다. 예시들은 선택지를 비교하기 위한 참고다.

백엔드의 AST, 바이트코드, VM, 객체 모델은 [백엔드 작업 문서](novi-object-oriented-backend.md)에 있다. 프런트엔드 표기를 결정한 뒤 파서와 의미 검사를 그 결정에 맞춘다.

Novi 언어로 작성한 소스 파일의 확장자는 `.nvi`다. 컴파일러/런타임 구현 파일은 C++ `.cpp`/`.hpp`로 유지한다. `novi path/to/file.nvi`는 파일 전체를 한 번에 파싱한다.

고차원 수치 계산, 자동미분, 기호수학, 논리·증명 계층의 분리 설계는 [수학 기능 설계 제안](novi-mathematics-design.md)을 참고한다. 그 문서의 예시 문법은 확정 전까지 제안으로 취급한다.

## 사용자가 결정한 항목

아래 표기는 현재 합의된 Novi 표면 문법이다. 의미 규칙이 비어 있는 부분은 임의로 확정하지 않고 후속 결정 항목에 남긴다.

| 기능 | 결정된 Novi 표기 | 남은 의미 규칙 |
|---|---|---|
| 표준 라이브러리 import | `import {novi} from {novi};` | `novi`가 기본 제공하는 함수 목록 |
| 사용자 모듈 import/export | `export {이름};` 및 `import {이름} from {./path.nvi};` | export 묶음, 경로 해석, 이름 충돌 처리 |
| Import alias | `import {} from {} like ('별명');`; `import {abc}=('별명');` | 두 alias 형태의 정확한 범위와 이름 규칙 |
| Export 이름 alias | `export {} like ("별명");` | export 묶음과 별명의 범위 |
| 산술 연산 | `+`, `-`, `*`, `/`, `%`, `//`, `mod(a,b)` | 문장 안에서 값 뒤의 `//`는 몫 연산자 |
| 논리 연산 | `and`, `or`; `&`, `|`도 같은 논리 연산 | truthiness를 검사하고 단락 평가하며 결과는 `boolean`으로 정규화 |
| 비트 연산 | `&&`, `||`, `^`, `~`, `<<`, `>>` | 정수 폭, 부호, shift 범위 |
| 대입·비교 | `=` 대입, `==` 같음, `!=`, 대소 비교, `is` 비교 | `is` 비교가 값 동등성·객체 동일성 중 무엇인지 |
| 정규식 lookahead | `?=`는 정규식 전방탐색에만 사용 | `regex("패턴")` 안에서만 토큰화하도록 제안 |
| 조건식 | `조건 ? 참일_때_값 : 거짓일_때_값` | 우선순위, 결과 타입 |
| 조건문 | `is (조건식) { ... }`; `also (조건식) { ... }`; `isnothing; { ... }` | 블록 종료, `else if` 연결 문법; `if/elif/else`도 별칭으로 허용 |
| 한 줄 조건문 | `is (조건식) then 명령;`; `if`도 같은 위치에 사용 | 여러 문장을 허용할지 |
| 조건 반복 | 조건 블록 안 `reflex(값) 변수;` | 조건이 참인 동안 재평가하는 반복으로 제안; 종료 방향과 값 전달 규칙 |
| 조건 반복 별칭 | `during (조건식) { ... }` | `while`과 같은 조건 반복 |
| 순회 반복 | `foreach (item in items) { ... }` | map 쌍은 `(key, value)`로 순회; 키/값 단독은 `keys()`/`values()` |
| 반복 제어 | `break;`, `continue;` | 중첩 시 가장 안쪽 루프에 적용; 루프 바깥은 문법 오류 |
| 수열 생성 | `range(start, step, stop)` | 정수 인수, 0이 아닌 간격, 종착값은 미포함; 현재 `foreach`는 range를 순회 |
| 수학 배열 | `tensor<T>`; optional shape `tensor<T, [2, 3]>` | 동질 원소, 동적 shape 기본, `vector`/`matrix`는 rank 별칭 |
| 배열 곱 | `+`/`-` 원소별; `.*`/`./` 원소별 곱/나눗셈; `matmul(a,b)` 선형대수 곱 | broadcast는 trailing axis에서 동일하거나 한쪽 크기 1일 때 |
| Boolean 검사 | `istrue(조건식)`은 `true` 또는 `false` 반환 | 조건식 타입 |
| 출력 | `console.print(값)` 또는 `console.print(값:출력타입)` | 타입 지정 시 출력 직전 변환; 미지정 시 원래 값 |
| 함수 선언 | `function NAME (인자) { ... }` | C식 사전 선언(prototype)을 별도 지원할지, 인자 타입/반환 타입 표기 |
| Lambda | `function {인자} then 명령;` | 표현식 반환, 여러 문장 lambda, 캡처 규칙 |
| 함수 반환 | `output(값)` 또는 `return(값)` | 둘 다 같은 반환 기능; 값 생략 허용 여부 |
| 클래스/공유 멤버 | `class`, `static`; Java/Kotlin과 유사한 클래스 모델 | 구체적 범위는 아래 클래스 항목에서 선택 |
| 메타데이터 | `@` | `@` 뒤 문법과 컴파일러가 처리할 metadata 목록 |
| 비동기 | `async`, `await` | 반환 타입, 오류 전파, 취소·스케줄러 규칙 |
| 접근 제어 | 클래스 기본 공개, `#` 또는 `private`는 비공개, `public` 명시 가능 | `#`의 멤버 접두 문법과 상속 접근 규칙 |
| 문장 종료/주석 | 문장 끝 `;`; `{ ... }`로 끝나는 문장은 생략 가능. 줄 시작 또는 `;` 뒤 `//`는 한 줄 주석, 여러 줄은 `//* ... *//` |
| 키워드 표기 | 키워드 동의어 허용, 키워드 대소문자 구분 없음 | 일반 식별자와 타입 이름은 대소문자 구분 여부를 별도 결정 |
| 예외 처리 | `try { ... } catch (error) { ... } finally { ... }`; `throw ErrorKind;` | 오류 payload/type |

변수와 문자열 표기는 다음과 같이 정했다.

```text
vari amount : integer = 3;         // 변경 가능한 변수, 타입 필수
let limit : integer = 10;          // 상수 선언, 타입 필수
vari amount : = integer(3);        // 타입 변환 초기화 표기
constant amount;                   // 기존 변수를 상수로 전환
vari first : string = 'text';      // 작은따옴표 문자열
vari second : string = "text";     // 큰따옴표 문자열
console.print(amount:string);       // 출력 때만 문자열로 변환
```

위 두 문자열 따옴표는 모두 문자열이다. 모든 선언에는 타입 표기가 필요하다. `vari`는 재할당 가능하고 `let`은 새 상수를 선언한다. `constant NAME;`은 이미 선언된 변수를 이후 변경할 수 없게 만든다. `console.print(value:type)`에서 타입을 생략하면 기존 값 표현을 출력하고, 지정하면 출력 시점에만 변환한다. `: = Type(value)`는 타입을 명시해 초기값을 변환하는 표기다.

Map 동작은 다음과 같이 결정했다. 없는 키 조회는 `null`을 반환한다. `foreach (key, value in map)`은 항목 쌍을 순회하고, 변수 하나로 map을 순회하면 키를 순회한다. 값만 순회할 때는 `map.values()`를 사용한다. `map.keys()`와 `map.values()`는 목록을 반환한다. 목록의 인덱스는 0부터 시작하며, 범위를 벗어난 목록 인덱스는 런타임 오류다. 현재 구현은 `dictionary()` 빈 값 생성, `map[key]` 조회·대입, key/value 순회를 제공한다. map literal 표기는 아직 정하지 않았다.

list·set·queue의 첫 기본 API는 문법 변경 없이 생성 함수와 메서드 호출로 제공한다. 이는 확정된 표기 원칙 위에 둔 초기 API 제안이며, 표준 라이브러리 정리 시 이름과 반환 규칙을 다시 확정한다.

```text
vari names : list = list();
names.append('Ada');
names[0] = 'Grace';
foreach (name in names) { ... }

vari tags : set = set();
tags.add('compiler');
tags.contains('compiler');

vari tasks : queue = queue();
tasks.enqueue('parse');
vari task : string = tasks.dequeue();
```

초기 동작은 list 삽입 순서 유지, set 중복 무시, queue FIFO 순서다. 이 컨테이너들은 `length()`를 제공한다. list는 `append`, `contains`, `remove`, `clear`를 제공하며, `remove(value)`는 첫 일치 항목만 지우고 성공 여부를 boolean으로 반환한다. 빈 queue의 `dequeue()`는 런타임 오류다. list·tuple·set·queue 순회는 시작 시점의 값을 복사한다. `tuple(a, b, ...)`는 가변 인수 생성자이며, tuple은 생성 뒤 인덱스 대입이 불가능하다. tuple literal과 구조 분해 문법은 후속 결정 항목이다.

`range(start, step, stop)`은 시작값부터 간격만큼 증가/감소하는 지연 수열을 만든다. Python의 range처럼 `stop`은 포함하지 않으며, `step`은 0일 수 없다. 음수 간격은 역방향 수열이다. 현재 실행기 첫 지원은 정수 인수 범위로 제한한다.

Novi 프로젝트는 `novi new <project-name>`으로 생성한다. 기본 구조는 `src/main.nvi`, `novi.toml`, `README.md`, `.gitignore`다. 프로젝트 이름은 영문자·숫자·`-`·`_`만 허용하며 같은 이름의 경로가 이미 있으면 생성하지 않는다.

```text
foreach (key, value in user) {
    console.print(key);
    console.print(value);
}
foreach (key in user.keys()) { ... }
foreach (value in user.values()) { ... }
```

현재 실행기에서 딕셔너리는 기본 제공 `dictionary()`로 빈 값을 만들고 대괄호로 값을 읽고 쓴다. 다음은 문법 예시다.

```text
vari user : dictionary = dictionary();
user['name'] = 'Novi';
console.print(user['missing'] == null); // true
```

딕셔너리 순회는 순회 시작 시 항목 순서를 복사한 뒤 그 순서를 돈다. 순회 중 map을 수정해도 현재 순회에는 새 항목이 추가되지 않는다. 내부 저장은 삽입 순서 벡터이며 key 탐색은 선형이므로, 큰 map 성능이 필요해지면 hash/index 구조로 바꾸고 측정한다.

사용자 모듈은 `export`로 내보낸 이름만 다른 파일에서 `import`할 수 있다. 기본 라이브러리는 컴파일러/런타임이 제공하므로 매번 `export` 선언을 작성하지 않는다. 기본 형태와 alias 예시는 다음과 같다.

```text
import {novi} from {novi};                  // 기본 라이브러리
import {abc} from {./module.nvi};             // export된 이름 가져오기
import {} from {./module.nvi} like ('별명');
import {abc}=('별명');
export {abc};
export {} like ("별명");
```

import/export 문장에는 세미콜론을 붙인다. 경로는 따옴표 없이 중괄호 안에 쓴다. `import {abc}=('별명')`가 가져온 `abc` 자체의 별칭인지, 모듈 이름의 별칭인지는 아직 정하지 않았다. `export {} like ("별명")`의 별명이 내보낸 심볼 묶음의 이름인지 모듈 이름인지도 정해야 한다.

일반 문장은 `;`로 끝낸다. 블록 `}`로 끝나는 제어문·함수·클래스 문장에는 세미콜론을 쓰지 않는다. 키워드는 대소문자를 구분하지 않으며 동의어 표기도 허용한다. 줄 시작의 `//` 또는 세미콜론 뒤의 `//`는 줄 끝까지 주석이다. 문장 안에서 값 뒤에 오는 `//`는 몫 연산자다. 여러 줄 주석은 `//*`로 시작해 `*//`로 닫는다.

산술 연산 기호는 `+`, `-`, `*`, `/`를 그대로 쓰기로 했다. 논리 연산자는 `and`, `or`다. 조건문은 `is`가 기본 표기이며 `if/elif/else`도 `is/also/isnothing` 대신 쓸 수 있다.

예시 모양:

```text
is (istrue(조건식)) {
    console.print("조건이 참");
} isnothing; {
    console.print("조건이 거짓");
}
```

`istrue(...)`는 조건을 검사해 `true`/`false` 값을 돌려주는 식이다. `is (...) { ... }`는 본문을 실행하는 조건문이다. `if/elif/else`도 각각 `is/also/isnothing` 자리에 쓸 수 있다. `console.print`는 기본 제공 출력 함수 호출이며, `output(...)`과 `return(...)`은 같은 반환 문장이다. 작은따옴표와 큰따옴표는 모두 문자열 리터럴이다.

한 줄 조건문은 조건이 참일 때 `then` 뒤의 명령 하나를 실행한다. 블록형과 함께 사용할 수 있다.

```text
is (a == 10) then console.print(a);
if (a == 10) then console.print(a);
```

조건식에서 `=`는 대입이므로 값의 같음을 검사할 때 `==`를 쓴다. 여러 명령이 필요하면 `{ ... }` 블록을 쓴다.

`reflex(value) target;`은 현재 조건문 블록에서 계산한 다음 값을 `target` 변수에 넣고 조건을 다시 검사하는 제어 구문으로 제안한다. 조건이 참인 동안 블록을 반복하고, 거짓이면 빠져나오는 형태다.

```text
is (a < 10) {
    console.print(a);
    reflex(a + 1) a;
}
```

위 예시는 `a`를 1씩 증가시키며 조건을 재검사한다. `reflex`를 실행하는 모든 경로가 다음 값을 제공해야 하는지, 여러 변수를 갱신할 수 있는지는 정해야 한다. 조건이 처음부터 거짓이면 블록과 `reflex`를 실행하지 않는다.

Lambda는 `function {인자} then 명령;` 형태의 익명 함수 식으로 제안한다.

```text
vari double : = function {x} then output(x * 2);
```

이 문법에서 `output(...)`은 lambda의 반환값을 정한다는 안이다. 반환 표현식 생략, 여러 문장, 캡처는 미정이다.

## 작성 규칙

- `Novi => ""`만 사용자가 채운다. 다른 언어 예시는 참고용이다.
- 표기뿐 아니라 **규칙**도 정한다. 예: 재할당 가능 여부, 스코프, 우선순위, 오류 조건.
- 키워드 문법과 기본 제공 함수를 구분한다. `console.print(...)`는 기본 제공 출력 함수 호출로 정했다.
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
| **Novi** | `vari NAME : TYPE = VALUE`, `let NAME : TYPE = VALUE`, 또는 `vari NAME : = TYPE(VALUE)` |

- `vari NAME : TYPE = VALUE`는 재할당 가능한 타입 지정 변수다.
- `let NAME : TYPE = VALUE`는 선언 시 값을 지정하는 상수다.
- `vari NAME : = TYPE(VALUE)`는 명시한 변환 형식으로 초기화한다.
- 선언에는 타입 또는 명시적 변환 형식이 필요하다.
- 기존 변수에 `vari` 없이 값을 재할당하는 축약 문법: **Novi => ""**
- 선언과 초기화 분리 및 초기값 생략: **Novi => ""**
- 변수 스코프(블록/함수/모듈/전역): **Novi => ""**
- 이름 가리기(shadowing) 허용 여부 및 처리: **Novi => ""**

현재 VM은 `boolean`, `string`, `integer`, `natural`, `nonint`, `realn`, `decimal`, `scalar`, `null`/`none`, `nan`, `list`, `tuple`, `dictionary`, `set`, `queue`, `range`의 선언 타입을 검사하며, 같은 타입 규칙을 재대입 때도 적용한다. `complex`, `imaginary`, exact `Rational`/`Irrational`, `vector`, `matrix`, `tensor`, `undefined`는 값 표현과 연산 규칙이 구현되기 전까지 지원 타입으로 처리하지 않는다. 이 단계는 실행 시 검사이며, 정적 의미 분석은 후속 단계다.

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
| **Novi** | `let NAME : TYPE = VALUE` (새 상수 선언); `constant NAME;` (기존 변수를 상수로 전환) |

- 상수가 객체 내부 필드까지 고정하는지, 변수 binding만 고정하는지: **Novi => ""**
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
- 기본 타입 이름과 의미: 아래 2.1.1 초안 목록을 기준으로 겹치는 분류와 표현 방식을 결정한다.

#### 1.3.1 제안된 타입 목록

아래는 사용자가 제안한 이름을 보존한 목록이다. 이름을 바꾸거나 제거하지 않았으며, 분류 관계·저장 표현·규칙은 아직 미정이다.

| 분류 | 제안된 Novi 타입 이름 | 결정해야 할 점 |
|---|---|---|
| 기본 값 | `boolean`, `string` | `boolean`의 표기 대소문자, 문자열 encoding/불변성 |
| 숫자 묶음 | `scalar` | 수치 타입의 공통 상위 분류/제약인지 실제 저장 타입인지 |
| 정수 계열 | `integer`, `nonint`, `natural` | `nonint`가 정수가 아닌 모든 수인지, integer/natural과의 포함 관계, bit width/부호/overflow, natural에 0 포함 여부 |
| 실수 계열 | `realn`, `decimal`, `nrc`, `rc` | `realn`의 정확한 뜻, 이진 부동소수점과 십진수 구분, 유한/무한 소수 구분 |
| 복소수 계열 | `complex`, `imaginary` | `complex(a,b)`에서 a=실수부·b=허수부, `imaginary`가 실수부 0인 복소수의 별칭/하위 타입인지 |
| 수학적 수 분류 | `Rational`, `Irrational` | 값의 수학적 속성인지 저장 표현인지, exact 계산 지원 여부 |
| 선형대수 | `vector`, `matrix`, `tensor` | rank/shape, 원소 타입, 연산 및 메모리 배치 |
| 컬렉션 | `list`, `tuple`, `queue`, `dictionary`, `set` | 변경 가능성, 순서, 인덱스/키/중복/동등성 |
| 특수 값 | `none`, `null`, `nan`, `undefined` | 각각의 의미 및 숫자 타입과의 관계 |

표기 예시로 `complex(a,b)`를 제안했다. 여기서 `a`와 `b`의 표기 단위는 숫자 값이며 각각 실수부와 허수부다. 이 문서 예시는 아직 수학 연산의 정밀도나 자료형 강제를 결정하지 않는다.

성능과 타입 체계를 위해서는 **저장 표현**과 **수학적 분류**를 분리하는 편이 좋다. 예를 들어 `integer`·`decimal`·`complex`는 값을 어떤 형식으로 저장/연산할지 정하는 타입 후보이고, `natural`·`Rational`·`Irrational`은 값의 수학적 성질 분류로 볼 수 있다. 한 값은 여러 성질을 동시에 가질 수 있으므로 이를 모두 서로 배타적인 런타임 타입으로 만들면 타입 검사와 연산 규칙이 복잡해질 수 있다.

- 유한 소수(decimal expansion)는 유리수이며, 무한 순환 소수(`rc`)도 유리수다. 무한 비순환 소수(`nrc`)는 실수의 십진 표기 속성이고 통상 무리수에 해당한다. `Rational`/`Irrational`은 저장 형식과 구분해서 다룰지 결정한다.
- 일반적인 유한 정밀도 binary float는 정확한 수학적 무리수를 보존하지 않는다. `Irrational` 값을 정확히 계산하려면 기호식/임의정밀도 방식이 필요하고, 근삿값 계산이면 정밀도와 오차 규칙을 정해야 한다.
- `nan`은 숫자 연산의 특수 값으로 둘지 독립 타입/값으로 둘지, `null`·`none`·`undefined`가 각각 결측·미초기화·반환 없음 중 무엇을 뜻할지 구별해야 한다.
- `vector`·`matrix`·`tensor`는 수치 연산을 위한 shape 있는 자료구조로, `list`·`tuple`·`queue`·`dictionary`·`set` 같은 일반 컬렉션과 원소 타입·연속 메모리·연산 규칙을 분리할지 결정한다.

- 이름과 대소문자(`boolean`/`Boolean`, `Rational` 등): **Novi => ""**
- `scalar`가 타입 생성자/상위 타입/namespace 중 무엇인지: **Novi => ""**
- `realn`의 정의와 `nonint` 추가 여부: **Novi => ""**
- scalar 계열 분류의 포함 관계와 상호 배타 여부: **Novi => ""**
- 명시적 형 변환의 추가 표기와 변환 실패 규칙: **Novi => ""**
- 유한 소수는 소수 자릿수 n에 대해 값×10^n 정수를 저장하고, 표시 시 scale을 적용하는 방식을 제안한다.
- 위 표현은 scaled integer와 decimal scale n을 함께 저장한다. 값이 커지면 scaled integer의 폭과 연산 비용이 늘어난다.
- 무한 순환 소수는 정수부·비순환 소수부·순환 주기와 주기 표시를 함께 저장하는 방식을 제안한다. 정확한 사칙연산을 위해서는 이 구조 외에 정규화된 분자/분모 표현을 함께 쓸지도 검토한다.
- 순환하지 않는 무한소수의 모든 자릿수를 유한 메모리에 정확히 저장할 수는 없다. 임의정밀도 근삿값과 오차 경계, 또는 기호식 표현 중에서 선택해야 한다.
- 반복소수는 반복부만 저장해 표기 자체를 보존할 수 있지만, 정확한 산술 연산을 위해 정규화한 분자/분모 형태가 필요할 수 있다.
- 무한 비순환 소수의 연산 정밀도·반올림 규칙: **Novi => ""**
- 숫자 크기에 따라 고정 폭 정수, 임의정밀도 정수, decimal scale 저장 중 선택하는 기준: **Novi => ""**

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
| 정수 | `42` | `42` | `42` | `42` | `42` | `42` (타입 선언 문법과 함께 결정) |
| 소수 | `3.14` | `3.14` | `3.14` | `3.14` | `3.14` | `3.14` (정밀도/타입 추론 미정) |
| 문자열 | `'hi'`, `"hi"` | `"hi"` | `"hi"` | `"hi"` | `'hi'`, `"hi"`, `` `hi` `` | `'hi'` 또는 `"hi"` |
| 문자 | 길이 1 문자열 | `'a'` | `'a'` | `'a'` | 별도 문자 타입 없음 | `=> ""` |
| 참/거짓 | `True`, `False` | `true`, `false` | `true`, `false` | `true`, `false` | `true`, `false` | `true`, `false` |
| null 값 | `None` | `null` | `nullptr` | `nil` (특정 타입의 nil) | `null`, `undefined` | `=> ""` |
| 배열/목록 | `[1, 2]` | `new int[]{1, 2}` / `List.of(1, 2)` | `std::array` / `std::vector` | `[]int{1, 2}` | `[1, 2]` | `=> ""` |
| 맵 | `{"a": 1}` | `Map.of("a", 1)` | `std::map` 초기화 | `map[string]int{"a": 1}` | `{"a": 1}` | `=> ""` |
| 집합 | `{1, 2}` | `Set.of(1, 2)` | `std::set` 초기화 | 표준 내장 집합 없음 | `new Set([1, 2])` | `=> ""` |
| 보간 문자열 | `f"Hi {name}"` | 문자열 연결/`String.formatted` 등 | 문자열 연결/`std::format` | `fmt.Sprintf` | `` `Hi ${name}` `` | `=> ""` |

- 정수 폭, 부호, 오버플로 처리: **Novi => ""**
- 부동소수점 종류와 정밀도: **Novi => ""**
- 정수 나눗셈의 결과 타입/0 나눗셈 처리와 Python식 음수 몫·나머지 권장안의 최종 확정: **Novi => ""**
- 문자열 결합과 비교는 구현 일부. 인덱스는 0부터 시작하며 UTF-8 코드포인트 하나를 반환하고, `.length()`도 코드포인트 수를 반환한다. 결합 문자 여러 개로 구성된 문자소 단위 처리는 후속 과제다.
- 배열/맵/집합의 변경 가능성, 순서, 동등성: **Novi => ""**
- null과 초기화되지 않은 값의 구분: **Novi => ""**

### 2.2 연산자 표기와 우선순위

| 기능 | Python | Java / C++ / Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|
| 산술 | `+ - * / // % **` | `+ - * / %` (C++ 외 언어별 차이) | `+ - * / % **` | `+`, `-`, `*`, `/`, `%`, `//`, `mod(a,b)` (확정) |
| 대입 | `=` | `=` | `=` | `=` (확정) |
| 복합 대입 | `+=`, `-=`, ... | `+=`, `-=`, ... | `+=`, `-=`, ... | `+=`, `-=` (확정) |
| 비교 | `== != < <= > >=` | 유사 | 유사 | `==`, `!=`, `<`, `>`, `<=`, `>=`, `is` (의미와 문맥 미정) |
| 논리 | `and or not` | `&& || !` | `&& || !` | `and`, `or`, `!`; `&`, `|`도 논리 별칭 (확정) |
| 비트 | `& | ^ ~ << >>` | 유사 | 유사 | `&&`, `||`, `^`, `~`, `<<`, `>>` (확정) |
| 증가/감소 | 없음 | `++ --` (Java/C++), Go는 별도 문장 | `++ --` | `=> ""` |
| 조건 선택 | `a if c else b` | `c ? a : b` | `c ? a : b` | `조건 ? 참일_때_값 : 거짓일_때_값` (확정) |
| null 대체/안전 접근 | 관용 패턴 | 언어·버전별 제공 | `??`, `?.` | `=> ""` |
| 타입 검사/변환 | `isinstance`, 생성자 | cast/`instanceof` 등 | `typeof`, `instanceof` | `=> ""` |

- 전체 연산자 우선순위 표: **Novi => ""**
- 결합 방향 및 단항·대입 연산 규칙: **Novi => ""**
- `and`/`or`와 별칭 `&`/`|`는 실행 단계에서 truthy/falsy를 검사해 boolean을 반환하고 단락 평가하도록 우선 구현했다. boolean만 허용할지 여부는 아직 확정하지 않았으며, 타입 검사기가 생기면 정적으로 제한할 수 있다.
- 객체의 `==`와 동일 객체 비교를 구분하는가: **Novi => ""**
- 위 truthy/falsy 지원 여부는 현재 구현 기본값이고, 최종 언어 규칙은 미정이다.
- 암시적 형변환, 숫자/문자열 혼합 연산 처리: **Novi => ""**
- 연산자 오버로딩 허용 여부: **Novi => ""**

#### 연산자 표기 충돌

- `=`는 대입, `==`는 같음 비교, `?=`는 정규식 전방탐색으로 확정됐다.
- `&`/`|`는 `and`/`or`의 논리 별칭이며, `&&`/`||`는 비트 연산이다. C 계열 관례와 반대이므로 parser, formatter, 진단 문구가 같은 규칙을 따라야 한다.
- `is`는 조건문 시작과 비교 양쪽에 쓰인다. `is (조건) {`처럼 block이 오는 문맥과 `left is right` 식 문맥을 파서에서 구분한다. 비교의 의미가 동등성인지 객체 동일성인지는 미정이다.
- 음수 몫·나머지는 Python처럼 `//`를 바닥 내림으로, `%` 결과 부호는 제수와 같게 하는 규칙을 권한다. `mod(a,b)`도 `%` 별칭으로 두면 의미가 하나로 정리된다.

### 2.3 비트 연산

| 연산 | 제안 표기 | 예시(정수 비트) | 남은 규칙 |
|---|---|---|---|
| AND | `&&` | `1010 && 1100` → `1000` | 정수 bit width와 부호 |
| OR | `||` | `1010 || 1100` → `1110` | 정수 bit width와 부호 |
| XOR | `^` | `1010 ^ 1100` → `0110` | 음수의 bit width/표현 |
| NOT | `~` | 폭이 고정되지 않으면 결과가 달라짐 | 정수 bit width와 signed 표현 |
| 왼쪽 이동 | `<<` | `0011 << 1` → `0110` | overflow 및 범위 초과 이동 |
| 오른쪽 이동 | `>>` | `0110 >> 1` → `0011` | signed 값의 산술/논리 이동 |

비트 연산이 산술 연산보다 항상 빠르다고 가정하지 않는다. 실제 성능은 타입, compiler, CPU에 따라 측정한다.

### 2.4 정규식 전방탐색

`?=`는 정규식 positive lookahead에만 쓴다. 경계는 `regex("패턴")` 기본 제공 함수 호출로 두는 것을 제안한다. `regex("[0-9]+(?=원)")`로 `"1000원"`을 검색하면 `"1000"`을 매치하고 `원`은 결과에 넣지 않는다. `(?=원)`만 쓰면 숫자를 선택하는 게 아니라 `원` 앞의 위치만 확인하므로 숫자 패턴도 필요하다.

### 2.5 다른 언어의 음수 나눗셈·논리 연산

정수 `-7`과 `3`의 나눗셈은 대표적으로 두 규칙이 있다.

| 규칙 | 몫 | 나머지 | 사용하는 언어 예 |
|---|---:|---:|---|
| 바닥 내림(floor) | `-7 // 3 = -3` | `-7 % 3 = 2` | Python. 나머지 부호가 제수와 같고 `a = (a // b) * b + (a % b)`를 만족한다. |
| 0 방향 절삭(truncate toward zero) | `-7 / 3 = -2` | `-7 % 3 = -1` | C++, Java, Go 정수 나눗셈. 나머지 부호가 피제수와 같다. |

또 다른 명확한 선택은 유클리드 나머지로, 항상 `0 <= r < |b|`를 만족시킨다. Novi에는 `//`를 몫 연산자로 이미 두었으므로, **`//`는 바닥 내림하고 `%`와 `mod(a,b)`는 제수 부호를 따르는 Python식 규칙**을 권한다. 그러면 `a = (a // b) * b + a % b`가 성립한다. 예: `-7 // 3 == -3`, `-7 % 3 == 2`. `%`와 `mod(a,b)`는 같은 연산으로 둘 수 있다.

논리 연산도 언어별 규칙이 다르다.

| 언어 방식 | 피연산자와 반환 | 평가 순서 |
|---|---|---|
| Python `and/or` | truthy/falsy 값을 받고 마지막으로 평가한 피연산자 값을 돌려줄 수 있다. | 왼쪽부터 평가하고 결과가 정해지면 오른쪽은 평가하지 않는 단락 평가 |
| C++/Java/Go `&&/||` | 조건형 boolean을 받고 boolean 결과를 낸다. | 왼쪽부터 평가하며 결과가 정해지면 오른쪽을 건너뛴다 |
| Novi 제안 | `and`와 `&`가 AND 별칭, `or`와 `|`가 OR 별칭; `&&`와 `||`는 정수 비트 연산 | 최적화를 위해 `and/or`와 `&/|` 모두 boolean 전용·단락 평가·boolean 반환으로 통일하는 안을 권한다. |

`&`/`|`를 `and`/`or`와 같은 연산으로 정했으므로 같은 피연산자 규칙, 우선순위, 단락 평가를 적용하는 편이 일관적이다. `&&`/`||`는 고정 폭 정수 또는 임의정밀도 정수의 비트 연산이며 width/음수 표현 규칙을 추가로 정해야 한다.

공식 참고: [Python 식과 연산자](https://docs.python.org/3/reference/expressions.html), [Java 연산자 명세](https://docs.oracle.com/javase/specs/jls/se26/html/jls-15.html), [Go 연산자 명세](https://go.dev/ref/spec#Operators), [C++ 곱셈 연산자 명세](https://eel.is/c++draft/expr.mul), [C++ 논리 연산자 명세](https://eel.is/c++draft/expr.log.and).

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
| **Novi** | `function NAME (인자) { ... }` |

- 반환 타입 생략 시 규칙: **Novi => ""**
- 반환 문장 표기: `output(값)` (확정)
- 반환값 생략 시 허용 여부와 결과: **Novi => ""**
- 재귀 호출 및 함수 이름의 스코프: **Novi => ""**

### 3.2 함수 기능

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 호출 | `f(x)` | `f(x)` | `f(x)` | `f(x)` | `f(x)` | `=> ""` |
| 기본 인수 | `def f(x=1)` | 오버로드 등 | 기본 인수값 | 직접 지원 안 함 | `function f(x=1)` | `=> ""` |
| 이름 있는 인수 | `f(x=1)` | 직접 지원 안 함 | 직접 지원 안 함 | 직접 지원 안 함 | 객체 인수 관용 | `=> ""` |
| 가변 인수 | `*args`, `**kwargs` | varargs `...` | parameter pack `...` | `...T` | `...args` | `=> ""` |
| 오버로딩 | 보통 단일 바인딩 | 시그니처 오버로딩 | 오버로딩 | 직접 지원 안 함 | 직접 지원 안 함 | `=> ""` |
| 익명 함수 | `lambda` | 람다 `->` | 람다 `[](...) {}` | 익명 `func(...) {}` | `(x) => x` | `function {인자} then 명령;` |
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
| 조건문 | `if cond:` / `elif` / `else` | `if (cond) { ... } else { ... }` (Python 외) | `is (cond) { ... }`; `also (cond) { ... }`; `isnothing; { ... }`; `is (cond) then command;` |
| 다중 분기 | `match` (버전별) | `switch`; Java/C++/Go/TS 지원 범위 다름 | `=> ""` |
| 조건식 | `a if cond else b` | `cond ? a : b` (Go에는 없음) | `=> ""` |
| 조건 반복 | `while cond:` | `while (cond) { ... }` | `during (condition) { ... }` 또는 `reflex(value) target;` |
| 기본 for | `for x in items:` | C 스타일 `for(init; cond; step)` | `=> ""` |
| foreach | `for x in items:` | Java enhanced for / C++ range-for / Go `for range` / JS `for...of` | `foreach (item in items) { ... }` |
| 반복 종료 | `break`, `continue` | `break`, `continue` | `=> ""` |
| 반복 else | Python `for/while ... else` | 일반적으로 없음 | `=> ""` |
| 패턴 분해 | tuple unpacking / `match` | 구조적 패턴 매칭은 언어·버전별 | `=> ""` |

- `istrue` 인자와 `is` 조건은 반드시 `boolean`이어야 하는가: boolean만 허용하는 안을 권한다.
- 들여쓰기 단위(공백 수/탭 금지/탭 폭)와 혼용 오류: **Novi => ""**
- 들여쓰기 감소로 블록이 끝나는 규칙: **Novi => ""**
- `else`와 else-if 표기: **Novi => ""**
- switch/match의 패턴 종류, fall-through, 기본 분기: **Novi => ""**
- 반복 대상과 iterator 프로토콜: **Novi => ""**
- 라벨 반복과 중첩 반복에서의 `break`/`continue`: **Novi => ""**
- 패턴 매칭의 exhaustiveness 검사: **Novi => ""**

### 4.2 오류 처리

| 기능 | Python | Java / C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|
| 예외 발생 | `raise Error(...)` | `throw new E(...)` / `throw E{...}` | 관례적으로 error 반환; `panic`도 있음 | `throw new Error(...)` | `throw ErrorKind;` |
| 예외 처리 | `try/except/finally` | `try/catch/finally` (Go 없음) | `if err != nil` | `try/catch/finally` | `try/catch/finally` |
| 오류 반환값 | 예외와 반환 모두 | 예외 중심 | `(value, error)` 관례 | 예외/Promise reject | `=> ""` |
| 오류 전파 | 예외가 상위로 전달 | 예외가 상위로 전달 | 명시적 반환 | 예외/Promise reject | `=> ""` |
| 자원 정리 | `with` / `finally` | try-with-resources / RAII | `defer` | `finally` / `using` (버전별) | `=> ""` |

현재 Novi 예외 표기는 `try { ... } catch (error) { ... } finally { ... }`이며, `catch`와 `finally` 중 하나 이상을 쓸 수 있다. `throw ErrorKind;`로 오류를 발생시키고 catch 변수에는 오류 메시지 문자열을 넣는다. `finally`는 정상 종료, catch 완료, 처리되지 않은 예외, `return`·`break`·`continue`에서 실행한다. 오류 종류는 이름 있는 타입/키워드로 정의하며, 예를 들어 `throw UndefinedVariable;`은 정의되지 않은 변수를 참조했을 때 사용할 오류 종류다. 구체적인 전체 오류 목록과 사용자 정의 오류 payload 문법은 후속 결정 항목이다.

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
| **Novi** | `class NAME { ... }` (기본 공개, 선언 문법 제안) |

클래스 멤버는 기본적으로 공개한다. `public`은 공개 상태를 명시하고, `private` 또는 멤버 앞의 `#`는 비공개로 표시하는 문법을 제안한다. `static` 멤버는 인스턴스가 아니라 클래스에 속한다.

```text
class Counter {
    static vari total : integer = 0;
    vari value : integer = 0;
    #vari secret : string = "hidden";
    private function reset() { value = 0; }
    public static function current() { output(total); }
}
```

`#`를 비공개 멤버 접두어로 쓰면 C 전처리 지시문과 같은 역할을 갖지 않는다. 외부 헤더 가져오기는 `#include` 대신 Novi `import`를 쓴다. `@`는 클래스·함수·멤버 바로 앞에 붙이는 메타데이터/속성 표기로 제안한다.

```text
@inline
function square(x) { output(x * x); }
```

`@inline` 같은 표식을 최적화 힌트로 쓸 수 있지만, compiler가 무시해도 프로그램 결과가 달라지지 않게 하는 방식을 권한다.

```text
async function loadData() {
    vari data : = await fetchData();
    output(data);
}
```

`async` 함수는 비동기 결과를 반환하고, 그 함수 안의 `await`는 완료될 때까지 현재 비동기 작업만 중단하는 표기로 제안한다. 차단 방식으로 스레드를 멈추지 않으며, 구체적 task/event-loop 모델은 백엔드 설계와 함께 정한다.

### 5.2 클래스 기능별 표기

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 생성자 | `__init__(self)` | 클래스명 메서드 | 클래스명 메서드 | 별도 생성자 없음/함수 관례 | `constructor()` | `=> ""` |
| 현재 인스턴스 | `self` | `this` | `this` | receiver 매개변수 | `this` | `=> ""` |
| 필드 접근 | `obj.name` | `obj.name` | `obj.name` | `obj.Name` | `obj.name` | `=> ""` |
| 상속 | `class B(A):` | `class B extends A` | `class B : public A` | 내장(embedding) | `class B extends A` | `=> ""` |
| 인터페이스 계약 | Protocol/ABC 등 | `implements I` | 추상 클래스/개념 | interface 암시 구현 | `implements I` | `=> ""` |
| 접근 제어 | 관례/이름 맹글링 | `public/private/protected` | 접근 지정자 | export 대문자 규칙 | `public/private/protected` | class 기본 `public`; `#` 또는 `private`는 비공개; 명시적 `public` 허용 |
| 정적 멤버 | `@staticmethod`/`@classmethod` | `static` | `static` | 타입 함수 관례 | `static` | `static` |
| 프로퍼티 | `@property` | getter/setter | 메서드 관례 | 메서드 관례 | getter/setter | `=> ""` |
| 오버라이드 표시 | 관례/데코레이터 | `@Override` | `override` | 메서드 재정의 대신 embedding | 재정의/데코레이터는 실험적 | `@Override` 형태 제안 |

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
| 코드 가져오기 | `import x`, `from x import y` | `import p.Type;` / module | `#include <...>` / C++20 `import` | `import "module/path"` | `import { x } from "m"` | `import {novi} from {novi};` |
| 공개 내보내기 | 기본 공개 | `public` | 선언/헤더 공개 관례 | 대문자 이름 | `export` | `export {이름};` |
| 별칭 가져오기 | `import x as y` | `import p.Type` | namespace alias | import alias | `import {x as y}` | `like ('별명')` 또는 `= ('별명')` (범위 미정) |
| 의존성 관리 | pip/uv/pyproject | Maven/Gradle | 패키지 관리자별 | Go modules | npm/pnpm/yarn | `=> ""` |
| 매크로/전처리 | 없음(외부 도구) | annotation processor | `#define`, `#if`, 템플릿 | `go generate`/build tags | 빌드 도구 변환 | `#`는 private 멤버 표기; 전처리 지시문은 미정 |
| annotation/decorator | decorator | annotation | attribute | 없음/주석 도구 | decorator | `@metadata` 표기 제안 |
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
| Novi | `@tag`, `#member` | `@` 메타데이터/최적화 힌트, 클래스의 `#` 비공개 멤버 | 두 표식의 적용 대상과 검증 규칙 |

- `#`는 비공개 클래스 멤버 표기로 정했으므로 `#include`/`#define` 계열 전처리 지시문과 겸용하지 않는다.
- Novi에 별도 전처리 단계가 필요한지, `@` 메타데이터를 어떻게 읽는지는 **Novi => ""**
- 매크로를 제공한다면 실행 가능 범위와 보안 경계: **Novi => ""**

### 6.2 C 헤더와 함수 선언 참고

```c
#include <stdio.h>

int add(int left, int right);  // 함수 선언(프로토타입)

int main(void) {
    printf("%d\n", add(2, 3));
    return 0;
}

int add(int left, int right) { // 함수 정의
    return left + right;
}
```

- `#include <stdio.h>`는 C 표준 입출력 헤더의 선언을 현재 소스에서 사용할 수 있게 한다. 예를 들어 `printf`, `scanf`, `FILE`, `stdin`, `stdout` 선언을 제공한다. 실제 함수 구현은 C 표준 라이브러리에서 링크된다.
- C에서 함수 호출 전에 선언/정의가 필요한 이유는 컴파일러가 함수 이름과 인자·반환 타입을 검사하기 위해서다. `add` 정의가 호출보다 앞에 있으면 별도 프로토타입은 필요 없다. 선언과 정의를 분리하거나 다른 파일에 둘 때 프로토타입을 쓴다.
- C는 파일 최상위의 문장을 차례로 실행하는 모델이 아니다. `main`이 진입점이고, 함수가 실행 흐름의 단위다. 모든 함수가 별도 선언문을 꼭 가져야 하는 것은 아니다.
- 현대 C에서는 선언이 보이지 않는 함수를 호출하는 방식에 의존하면 안 된다. 컴파일러는 헤더를 읽어도 함수 구현 자체는 보통 별도 표준 라이브러리에서 링크한다.
- Novi는 C 헤더 대신 `import {novi} from {novi}`로 표준 입출력 함수 선언을 가져오는 방식이다. 실제 표준 함수 이름/목록과 module-member 구성은 추가로 정한다.

## 7. 동시성, 비동기, 메모리

| 기능 | Python | Java | C++ | Go | JavaScript / TypeScript | **Novi** |
|---|---|---|---|---|---|---|
| 비동기 선언 | `async def` | `CompletionStage`/가상 스레드 등 | coroutine 라이브러리/표준 기능 | goroutine 호출 | `async function` | `async function NAME(...) { ... }` |
| 대기 | `await` | `.get()` 등 | `co_await` | 채널/동기화 | `await` | `await` |
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
| 출력 | `print(x)` | `System.out.println(x)` | `std::cout << x` / `std::print` | `fmt.Println(x)` | `console.log(x)` | `console.print(x)` (확정) |
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
| 변수/상수 | 대입 / 대문자 관례 | 타입 / `var`, `final` | 타입 / `auto`, `const`, `constexpr` | `:=`, `var`, `const` | `let`, `const`, `var` | JS + 타입 표기 | `vari`, `constant` |
| 함수 | `def`, `lambda` | 반환 타입·메서드, 람다 `->` | 반환 타입·람다 | `func` | `function`, `=>` | JS + 타입 표기 | `function`, `output`, `return` |
| 타입/클래스 | `class`, `type` | `class`, `interface`, `enum`, `record` | `class`, `struct`, `enum`, `template` | `type`, `struct`, `interface` | `class` | JS + `type`, `interface`, `enum` | `class` |
| 분기/반복 | `if`, `match`, `for`, `while` | `if`, `switch`, `for`, `while` | `if`, `switch`, `for`, `while` | `if`, `switch`, `for`, `range` | `if`, `switch`, `for`, `while` | JS + 타입 narrowing | `is`/`if`, `also`, `isnothing`, `reflex` |
| 오류/자원 | `raise`, `try`, `except`, `with` | `throw`, `try`, `catch`, `finally` | `throw`, `try`, `catch`, RAII | `error`, `defer`, `panic`, `recover` | `throw`, `try`, `catch`, `finally` | JS + 타입 검사 | `=> ""` |
| 모듈 | `import`, `from`, `as` | `package`, `import`, `module` | `#include`, `import`, `module` | `package`, `import` | `import`, `export` | JS + type-only import/export | `import`, `export`, `like` |
| 매크로/메타 | decorator | annotation | `#define`, template, `constexpr` | compiler directive comments | 표준에는 없음 | triple-slash directive/type operator | `@` metadata, `#` private 멤버 |
| 비동기 | `async`, `await` | 비동기 API | coroutine | goroutine | `async`, `await` | JS 기반 | `async`, `await` |
| 클래스 접근 | 관례/이름 맹글링 | `public/private/protected` | 접근 지정자 | 대문자 공개 규칙 | `public/private/protected` | JS 기반 | 기본 `public`, `#`/`private` |

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

## 13. 성능 목표 결정

Novi의 최우선 목표는 **최적화**다. 여기서는 “빠르게”라는 표현을 측정 가능한 실행·컴파일·메모리 목표로 구체화한다. 목표 수치와 대표 프로그램은 언어 기능을 함께 정할 때 합의한다.

- 우선 최적화할 대상(실행 시간/컴파일 시간/메모리/시작 시간/에너지): **Novi => ""**
- 비교 기준 언어·컴파일러·실행 모드: **Novi => ""**
- 대표 워크로드(수치 계산/문자열/객체/컬렉션/서버 등): **Novi => ""**
- 기준 하드웨어·운영체제·빌드 설정: **Novi => ""**
- 성능 목표 및 허용 회귀 폭: **Novi => ""**
- VM, AOT, JIT 중 첫 출시 기본 경로: **Novi => ""**
- 컴파일 시간과 실행 성능이 충돌할 때 우선순위: **Novi => ""**
- 런타임 메모리·GC pause 목표: **Novi => ""**
- 플랫폼별 Assembly가 허용되는 조건과 지원 CPU: **Novi => ""**
- 최적화 정확성 보장 방식(overflow, 부동소수점, undefined behavior 등): **Novi => ""**

최적화 전후 결과는 같은 코드·입력·하드웨어·빌드 설정으로 비교한다. 프로파일 없이 추측으로 넣은 최적화는 성능 개선으로 기록하지 않는다.

## 다음 결정 순서

이미 결정된 변수·문자열·import/export·연산자·조건/반복·클래스 접근 표기는 유지한다. 파서와 실행기를 설계하기 전에 남은 핵심 선택은 다음과 같다.

1. **연산 규칙**: 우선순위, `/`의 결과 타입과 0 나눗셈, `is` 비교의 정확한 의미, 비트 정수 폭.
2. **수치 정확성**: 정수 폭/오버플로, decimal 반올림, 무한소수 근사 정밀도와 경계.
3. **블록·문장 경계**: 중괄호 기반 조건문/함수와 들여쓰기의 역할, 세미콜론/줄바꿈 규칙.
4. **함수 형태**: 인자·반환 타입 표기, C식 prototype과 정의의 관계, lambda 반환·캡처 규칙.
5. **모듈 의미**: export 선언의 단위, import 경로, 두 alias 형태 중 이름이 바뀌는 대상.
6. **클래스·속성**: 생성자/상속/오버라이드, `@` 메타데이터 의미, private 멤버 상속 규칙.
7. **비동기**: `async` 반환 타입, `await` 문맥, task/event-loop, 취소와 오류 전파.
8. **기본 값 규칙**: `null`, `none`, `nan`, `undefined`의 구분과 컬렉션 리터럴 표기.

각 결정을 내리면 Novi 예시 코드를 추가하고, 백엔드 task 문서와 AST/바이트코드 설계를 함께 갱신한다.

## 참고 문법 자료에서 얻은 설계 메모

다음은 사용자가 제공한 문법 자료에서 Novi 설계에 참고할 만한 차이다. 이는 Novi 문법의 자동 채택 목록이 아니다.

| 주제 | 참고 자료의 포인트 | Novi 설계에 적용할 점 |
|---|---|---|
| Python 함수 | 일반 함수와 한 줄 lambda를 구분하고, 함수값을 변수에 담거나 다른 함수에 전달할 수 있다. 기본 인자와 가변 인자(`*args`, `**kwargs`)도 제공한다. | 우선 선언 함수와 사용자가 정한 `function {인자} then 명령;` lambda를 구현한다. 기본/가변 인자는 별도 기능으로 정한다. lambda를 값으로 쓸지와 반환 규칙을 명세한다. |
| Python 모듈 | 모듈은 파일 단위 이름공간이며 import는 모듈 또는 그 안의 이름을 참조한다. | Novi는 사용자가 정한 명시적 `export`와 import/export의 세미콜론을 따른다. 경로와 alias 해석, 내보낸 이름의 공개 규칙을 정한다. |
| Kotlin 함수·lambda | 함수와 lambda를 값으로 전달·반환하는 고차 함수 사용이 간결하다. 마지막 인자가 lambda인 호출은 중괄호를 괄호 밖으로 빼는 trailing lambda 표기가 있다. | Novi의 현재 lambda 표기를 유지하고, 함수 인자로 전달하는 호출 문법과 캡처 규칙은 나중에 정한다. 암시적 `it`/`this`는 초기 문법에 넣지 않는 편을 권한다. |
| JavaScript async | `async` 함수는 Promise로 감싼 결과를 반환하고 `await`는 완료된 값을 받는다. `await`는 실행 스레드를 막지 않고 현재 비동기 작업을 양보한다. | Novi의 `async` 결과는 `Task<T>` 같은 비동기 타입으로 표현하고, `await`는 async 문맥 안에서만 허용하는 안을 권한다. 취소와 오류 전파는 명세가 더 필요하다. |
| Java 클래스 | 클래스/파일·package/import 구조가 있고, `static`은 인스턴스가 아닌 클래스에 속한다. 접근 제어자는 정보 은닉을 위해 사용한다. | Novi는 사용자가 정한 클래스 기본 공개, `#`/`private` 비공개, `public` 명시, `static` 공유 멤버를 따른다. Java의 파일명과 public class 강제는 별도 결정 전에는 채택하지 않는다. |

참고 링크: [Python 모듈](https://namu.wiki/w/Python/%EB%AC%B8%EB%B2%95/%EB%AA%A8%EB%93%88), [Python 함수](https://namu.wiki/w/Python/%EB%AC%B8%EB%B2%95/%ED%95%A8%EC%88%98), [Kotlin 문법](https://namu.wiki/w/Kotlin/%EB%AC%B8%EB%B2%95), [JavaScript 문법](https://namu.wiki/w/JavaScript/%EB%AC%B8%EB%B2%95), [Java 문법](https://namu.wiki/w/Java/%EB%AC%B8%EB%B2%95).

현재 확인 과정에서 Python 함수·JavaScript·Kotlin·Java 문법은 미러/검색 색인으로 내용을 확인했다. Python 모듈 개별 문서는 미러에서 404로 열리지 않아, 해당 항목은 Python 문법 자료의 모듈 설명과 일반적인 모듈 구조를 참고한 요약으로 표시한다.
