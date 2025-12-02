# tetris

25년 2학기 객체지향프로그래밍 프로젝트

## Refactoring
- [x] show_gameState()을 gameState class에 포함시키기.
- [x] STAGE 내부에 배열을 가지고 있는 형식으로 수정.
- [x] show_logo() 에서 block을 랜덤하게 생성하는 로직을 BlockGenerator에게 위임
- [x] 블럭 나오는 순서가 항상 같음

## Improvement
- [x] "spacebar"을 통해 맨 아래로 내리기 (hard_drop)

## new Function
- [ ] 일시정지
- [ ] 게임종료
- [x] 떨어질 위치 미리보기
- [x] .txt 파일 형태로 고득점 저장 및 불러오기 (싱글용)
- [x] Single: 점수판, Human vs AI, Human vs Human 3개의 기능 나누기
- [ ] MultiPlay에서 상대방에게 공격하는 행위 (클리어 된 줄에서, 각 줄의 랜덤으로 1칸 빼기)

## Bug
- [ ] 점수판이 아직 안나옴
- [ ] scores.txt가 덮어씌어지면서 저장역할을 하지 못함
- [ ] 맨 위에서 블럭을 사라지게 하는 버그가 존재 (원인불명)

## 커밋 메시지 컨벤션

이 웹사이트는 [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) 규약을 따릅니다.

커밋 메시지는 [husky와 commit lint](https://theodorusclarence.com/library/husky-commitlint-prettier)를 사용하여 검사하며, 아래의 적절한 형식을 사용하지 않으면 커밋할 수 없습니다.

### 형식

`<타입>(<선택적 스코프>): <설명>`
예시: `feat(pre-event): add speakers section`

### 1. 타입

사용 가능한 타입은 다음과 같습니다:

- **feat** → 기능 추가 또는 제거에 대한 변경 사항. 예: `feat: add table on landing page`, `feat: remove table from landing page`
- **fix** → 버그 수정. 예: `fix: illustration overflows in mobile view`
- **docs** → 문서 업데이트 (README.md)
- **style** → 코드 로직에 영향을 주지 않는 스타일 업데이트 (임포트 순서 변경, 공백 수정, 주석 제거 등)
- **chore** → 새 의존성 설치 또는 의존성 버전 업데이트
- **refactor** → 코드 변경. 결과(output)는 동일하지만 접근 방식이 다름 (예: 리팩토링)
- **ci** → Github 워크플로우, husky 등 CI/CD 관련 설정 업데이트
- **test** → 테스트 스위트, cypress 파일 등 테스트 관련 파일 업데이트
- **revert** → 커밋을 되돌릴 때
- **perf** → 성능 관련 수정 (상태 파생, memo, callback 사용 등)
- **vercel** → Vercel 배포를 트리거하기 위한 빈 커밋. 예: `vercel: trigger deployment`

### 2. 스코프 (선택 사항)

페이지별 레이블. 예: `feat(pre-event): add date label`

\*스코프가 필요하지 않은 경우 작성하지 않아도 됩니다.

### 3. 설명

설명은 수행된 작업을 완전히 설명해야 합니다.

중대한 변경 사항(significant change)이 있는 경우 설명에 `BREAKING CHANGE`를 추가합니다.

**변경 사항이 여러 개인 경우, 하나씩 커밋합니다.**

- 콜론(:) 뒤에는 항상 하나의 공백이 있어야 합니다. 예: `feat: add something`
- `fix` 타입을 사용할 때는 이슈를 명시합니다. 예: `fix: file size limiter not working`
- 명령형, 현재 시제를 사용합니다: "change" (O), "changed" (X), "changes" (X)
- 문장 첫 글자를 대문자로 쓰지 않습니다.
- 문장 끝에 마침표(.)를 찍지 않습니다.
