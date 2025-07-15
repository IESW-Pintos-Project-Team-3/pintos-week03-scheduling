# Pintos Project 3: Scheduling implementation

## Lottery 스케줄러 구현

- **티켓 기반 스케줄링**: 각 쓰레드에 티켓 수를 할당하여, 전체 티켓 중 무작위로 선택된 티켓의 소유 쓰레드가 CPU를 점유합니다.
- **랜덤 함수 사용**: Pintos 내장 랜덤 함수를 활용하여 티켓 추첨을 수행합니다.
- **쓰레드 구조체 확장**: `struct thread`에 `total_ticket`과 `my_ticket` 필드를 추가하여 티켓 정보를 관리합니다.
- **티켓 범위 및 기본값**: 티켓 수의 최소/최대 범위와 기본값을 설정하여 우선순위 조정이 가능합니다.
- **리스트 관리**: 정렬 리스트와 비정렬 리스트를 각각 구현하여 성능을 비교합니다.
- **RB Tree 적용 고려**: 티켓 관리의 효율성을 위해 RB Tree 자료구조 적용을 검토하였습니다.

## Stride 스케줄러 구현

- **Stride 알고리즘**: 각 쓰레드에 stride 값을 할당하고, 최소 pass 값을 가진 쓰레드를 선택하여 CPU를 점유합니다.
- **우선순위와 starvation 방지**: 2의 지수승 stride 값과 일반 정수 stride 값을 비교하여 starvation 및 연산 속도를 테스트합니다.
- **쓰레드 구조체 확장**: `struct thread`에 `stride`와 `pass` 필드를 추가하여 스케줄링 정보를 관리합니다.
- **RB Tree 적용 고려**: pass 값 기준으로 쓰레드를 정렬하기 위해 RB Tree 자료구조 적용을 검토하였습니다.

## 테스트 및 평가

- Common : Fairness 측정 
- Lottery: 정렬 리스트 vs 비정렬 리스트 성능 비교
- Stride: pass 0일 때 독점 확인 
- 두 스케줄러 모두 RB Tree 적용 시 성능 및 효율성 평가
- 고정 소수점/부동 소수점 쓰지 않는 이유 설명 (보류) 
---