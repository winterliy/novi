# Novi 수학 기능 설계 제안

> 상태: 혼합형 방향 및 아래 핵심 기본안은 사용자 승인. 세부 numeric kind와 오류 목록은 추가 결정이 필요하다.

## 목표

Novi를 일반적인 프로그램도 작성할 수 있으면서 수치 계산, 고차원 배열, 기호 수학, 논리적 추론을 자연스럽게 표현하는 언어로 발전시킨다. 언어와 컴파일러의 최우선 목표인 최적화를 유지하되, 최적화는 계산의 수학적 의미와 사용자가 선택한 정밀도를 바꾸지 않아야 한다.

선택된 큰 방향은 **혼합형**이다. 배열·수치 계산을 기본으로 제공하고, 자동미분·기호 계산·증명 기능은 서로 분리된 수학 계층으로 구성한다. 이 구분은 NumPy의 균질 배열과 shape/dtype 모델, Julia의 여러 인자 타입 기반 디스패치, JAX의 함수 변환, SymPy의 기호 계산, Lean의 증명 커널에서 참고한 설계 원칙을 합친다.

## 다른 도구에서 취할 설계 원칙

| 참고 | Novi에 가져올 개념 | 복제하지 않을 것 |
|---|---|---|
| Julia | 함수와 연산을 모든 인자 타입에 따라 선택하는 다중 디스패치 | 동적 타입/JIT에 의존하는 전체 실행 모델 |
| MATLAB | 배열과 행렬을 수학 계산의 기본 표면으로 다루는 편의성 | 1부터 시작하는 색인. Novi는 이미 정한 0 기반을 유지 |
| Fortran | 수치 정밀도(kind)와 배열 rank/shape를 명시적으로 다루는 모델 | 컴파일러별 kind 번호를 사용자 문법에 노출 |
| NumPy | 균질 N차원 배열, dtype, shape, broadcasting | Python 객체 배열을 기본 수치 배열과 혼합 |
| SciPy | 희소 행렬, 선형대수, 적분, 최적화, 신호 처리 같은 도메인 도구 | 모든 과학 알고리즘을 Novi 핵심 문법에 넣기 |
| JAX | `grad`, 벡터화, 컴파일 같은 조합 가능한 함수 변환 | 자동 변환이 모든 동적·부수효과 코드를 처리한다고 가정 |
| SymPy | 기호식, 가정, 미분·적분·방정식 풀이 | 기호식을 일반 실수/부동소수점 값과 같은 저장 타입으로 취급 |
| Lean | 명제와 증명을 기계 검증하는 별도 층 | 첫 구현부터 의존 타입/증명 검증을 일반 런타임에 강제 |

참고 문서: [Julia 타입과 디스패치](https://docs.julialang.org/en/v1/manual/types/), [MATLAB 배열](https://www.mathworks.com/help/matlab/matrices-and-arrays.html), [Fortran 배열 rank](https://fortran-lang.org/learn/intrinsics/array/), [Fortran 수치 정밀도](https://fortran-lang.org/learn/best_practices/floating_point/), [NumPy 배열](https://numpy.org/doc/stable/reference/arrays.html), [SciPy 희소 배열](https://docs.scipy.org/doc/scipy/tutorial/sparse.html), [JAX grad/vmap/jit](https://docs.jax.dev/en/latest/101/transformations.html), [SymPy 기호 계산](https://docs.sympy.org/latest/tutorials/intro-tutorial/intro.html), [Lean 기초](https://lean-lang.org/faq/).

## 타입 설계

수학적 분류, 메모리 저장 형식, 배열 모양은 서로 다른 타입 축으로 둔다. 한 이름이 이 세 역할을 동시에 맡지 않도록 한다.

### 수치 영역과 저장 형식

- **정수:** 고정 폭 정수(kind)와 임의 정밀도 정수를 구분한다. 고정 폭은 정확한 범위와 오버플로 정책을 가진다.
- **유리수:** 분자/분모 기반 exact 값이다. 정규화와 0으로 나눔은 타입 규칙으로 처리한다.
- **십진수:** 유한 십진수를 scale 있는 정수 또는 동등한 exact decimal 표현으로 저장한다. 금융·십진 입력에서 binary float와 구별한다.
- **실수 근삿값:** binary floating point이며 `single`, `double`, 선택 가능한 확장 정밀도를 구별한다. 무리수 자체를 유한 비트로 exact 저장한다고 주장하지 않는다.
- **복소수:** 실수부와 허수부가 같은 정밀도 kind를 가진 수치 타입이다.
- **수학적 집합 제약:** `natural`, `rational`, `irrational`, `positive` 등은 저장 형식이 아니라 값에 대한 제약/가정으로 다룬다.
- **특수 값:** `nan`, `infinity`, `null`은 숫자 집합의 하위 타입으로 섞지 않고 명확한 값/오류 규칙을 갖는다.

현재 `integer`, `nonint`, `realn`, `decimal`, `nrc`, `rc`, `Rational`, `Irrational`, `natural`, `complex`, `imaginary` 이름은 보존할 수 있지만, 이들을 위 축에 배치하고 의미가 겹치는 이름은 별칭/제약/저장 타입 중 무엇인지 결정해야 한다. 특히 `nonint`는 정확한 집합 의미를 정하기 전까지 핵심 타입으로 고정하지 않는다.

### 벡터·행렬·텐서

수치 배열은 **원소 dtype + rank + shape + layout**으로 설명한다.

- `vector`는 rank 1, `matrix`는 rank 2, `tensor`는 임의 rank의 의미상 별칭으로 제안한다.
- 배열 원소는 기본적으로 동질적이다. 일반 `list`와 과학 계산용 tensor를 구별한다.
- 배열 shape는 컴파일 시 알려진 경우 정적 검사/특수화에 쓰고, 실행 때만 알려진 경우 런타임 shape 검사로 처리한다.
- 채택 표기: `tensor<realn, [2, 3]>`처럼 선택적으로 정적 shape를 타입에 붙이고, 기본 `tensor<realn>`은 실행 시 shape를 가진 배열로 쓴다.
- dense와 sparse는 dtype/rank와 별도의 저장/계산 레이아웃 선택이다. 희소 형식은 처음에는 `sparse(...)` 생성자 또는 수학 라이브러리 타입으로 제공한다.
- broadcasting은 shape가 호환될 때만 허용하고, 호환되지 않으면 조용한 절단/확장이 아니라 오류를 낸다.

## 연산 의미

사용자는 `*`, `+`, `-`, `/`, `and`, `or`, `&`, `|` 등 현재 연산 표기를 정했다. 수학 제안의 핵심 기본안도 채택했으며 구체적인 문법은 아래와 같다.

- 스칼라 산술은 기존 수학 연산을 따른다.
- 배열 `+`/`-`는 shape 호환 검사를 하고 원소별로 계산한다.
- `.*`, `./`, `.^`는 명시적인 원소별 연산 표기로 한다.
- 선형대수 곱은 `matmul(a, b)`로 명시한다. `*`는 기존 산술과 스칼라 스케일링을 유지한다.
- 고차원 수축(dot/einsum 계열)은 별도 함수 `contract` 또는 `einsum`으로 명시한다.
- 자동 broadcasting은 NumPy/JAX처럼 trailing axis 호환 규칙을 따른다. 원소별 연산과 broadcasting 모두 형상 정보를 보여주는 진단을 낸다.
- 논리값 배열(mask) 연산은 스칼라 명제 논리와 구분한다. `and/or`는 스칼라 논리, mask의 원소별 논리는 별도 연산/함수로 둔다.

이 연산자 표기는 기존 사용자가 정한 논리·비트 연산자와 일부 겹칠 수 있다. mask의 원소별 논리 연산자는 기존 operator 의미를 변경하지 않고 별도 함수/namespace로 제공한다.

## 일반화된 수학 연산

수학 연산은 구체 타입마다 별도 이름을 만들기보다, 공통 연산/제약을 구현 타입에 맞게 선택한다. 예를 들어 `+`는 정수·유리수·복소수·벡터·행렬에서 각 도메인 규칙을 가지며, 선택은 모든 인자 타입과 shape를 기반으로 한다.

초기 개념 제약:

- `Numeric`: 숫자 산술 가능
- `Ordered`: 대소 비교가 정의됨 (복소수는 일반적으로 미구현)
- `Exact`: 연산이 exact 값을 보존
- `TensorLike`: dtype, rank, shape 조회 가능
- `Differentiable`: 선택한 수치 체계에서 미분 가능
- `Logical`: 명제 조합 가능

이들은 우선 내부 타입/연산 규칙으로 구현하고, 사용자 정의 trait 문법은 제네릭/클래스 문법이 성숙한 뒤 노출한다. 라이브러리 함수 dispatch는 overload가 애매한 경우 컴파일 오류로 알린다.

## 수학 계층과 함수군

### 기본 수치·배열 계층

`math`, `tensor`, `linear` 같은 모듈 경계를 제안한다. 기본 수학 상수/삼각함수/지수로그, 배열 생성·shape 조회·reshape·transpose, 행렬 곱·분해·고유값·선형 시스템 풀이를 우선한다.

### 과학 수치 계층

SciPy에서 영감을 받아 `sparse`, `integrate`, `optimize`, `statistics`, `signal`, `interpolate` 같은 모듈로 분리한다. 수치 적분, 미분방정식, root/최적화, sparse linear solver, FFT·filter는 핵심 문법이 아니라 표준 또는 선택형 라이브러리다.

### 자동미분·벡터화 계층

JAX의 조합 가능한 변환을 참고해 다음을 함수에 적용하는 API로 제안한다.

- `grad(f)` / `jacobian(f)` / `hessian(f)`
- `vmap(f)` 또는 동등한 batch/vectorize 변환
- `jit(f)`는 계산 결과를 바꾸지 않는 컴파일 힌트

초기 자동미분은 순수 함수와 지원하는 실수/복소 tensor 연산으로 제한한다. 분기/반복·mutable state·외부 입출력에 대한 미분 규칙은 별도로 정의한다.

### 기호수학 계층

SymPy처럼 기호 변수와 식 AST를 수치 값과 구분한다. 후보 연산에는 simplify, substitute, diff, integrate, limit, series, solve가 있다. 기호 표현을 일반 `realn` 값으로 암묵 변환하지 않는다. 심볼 가정은 `positive`, `integer`, `real` 같은 논리 속성으로 저장하며, 결정 불가를 거짓으로 오해하지 않는다.

### 논리·증명 계층

일반 `boolean`은 실행 중 참/거짓 값으로 유지한다. 수학 명제는 `proposition` 같은 별도 표현으로 두고, `true`, `false`, `unknown`의 판정 결과를 구별한다. `unknown`은 `null`과 같은 결측값이 아니다.

Lean에서 영감을 받되, 정리·증명 기능은 일반 실행 언어의 필수 타입 시스템에 바로 넣지 않고 독립적인 proof 모드/라이브러리로 단계적으로 제공한다. 처음에는 명제 표현과 SMT/규칙 기반 증명 연결까지만 설계하고, dependent type 및 kernel checker는 별도 하위 프로젝트로 둔다.

## 최적화 백엔드 원칙

1. 파서 뒤에 타입·rank·shape 검사기를 둔다. 정적으로 알 수 없는 shape는 런타임 검사로 넘긴다.
2. 계산식을 수치 tensor IR로 내린다. scalar, dense tensor, sparse tensor 연산을 표현하고 source span을 보존한다.
3. 먼저 상수 접기, 공통 부분식 제거, loop/broadcast fusion, 임시 배열 제거, 순수 함수 특수화를 구현한다.
4. 배열 layout, SIMD, 멀티스레드, GPU/device 배치는 수학 의미와 분리된 backend 선택으로 둔다. CPU 경로와 결과 비교가 가능한 구현을 먼저 만든다.
5. 자동미분은 IR 연산별 derivative rule로 만들고, symbolics는 별도의 exact-expression IR를 사용한다.
6. 성능 향상은 실행시간/메모리 측정으로 확인하며 `@` 최적화 힌트만으로 성능 보장을 약속하지 않는다.

## 구현 단계 제안

1. **핵심 수치 계약:** 수치 kind, 정확/근삿값, promotion, overflow, division/error 규칙 확정.
2. **shape-aware tensor:** 생성/색인/shape 검사, vector/matrix 별칭, broadcasting, elementwise/linear algebra 구분.
3. **기본 linear algebra와 sparse:** 대표 dense kernels와 sparse format을 추가하고 external C/C++ backend 연결점을 둔다.
4. **최적화 IR:** shape specialization, fusion, scalar/vector kernel lowering.
5. **기호·자동미분:** 별도 symbolic IR와 derivative transformation.
6. **논리·정리 증명:** proposition semantics를 고정한 뒤 proof tooling을 독립 연결.

## 사용자가 정할 프런트엔드 항목

아래는 아직 남은 frontend 결정이다. tensor와 핵심 연산 표기는 승인된 기본안으로 기록했다.

1. optional fixed extents를 `tensor<realn, [2, 3]>` 말고 다른 표기로 쓸지.
2. `contract`/`einsum` 중 고차원 수축 기본 API를 무엇으로 할지.
3. 복소수·정밀도 kind·유리수·십진수 literal을 어떻게 표기할지.
4. 기호 변수 선언과 `diff`/`grad`/`integrate` 표기를 어떤 식으로 노출할지.
5. `boolean`, elementwise mask, 수학 명제, `unknown`의 문법 및 연산자를 어떻게 구분할지.
6. 오류 종류 이름을 `ShapeMismatch`, `PrecisionLoss`, `DivisionByZero`, `NonDifferentiable` 등으로 정할지.
7. `natural`, `rational`, `irrational`, `nonint`, `realn`, `decimal`, `rc`, `nrc`가 저장 타입인지 제약인지 별칭인지 배치할지.

## 설계 검토 기준

- 일반 Novi 코드는 수학 라이브러리를 쓰지 않아도 계속 사용할 수 있다.
- 자료형 이름은 exactness와 근삿값의 차이를 감추지 않는다.
- 텐서 shape 오류를 가능한 한 컴파일 시점에 설명하고, 동적 shape는 실행 오류에 실제 shape를 포함한다.
- scalar logical condition과 boolean tensor mask를 혼동하지 않는다.
- 자동미분·기호 계산·증명은 동일 계산 기능인 것처럼 뭉치지 않고, 각각 정의된 의미와 제한을 가진다.
- 최적화는 명시된 정밀도/오차 계약과 프로그램 결과를 보존한다.
