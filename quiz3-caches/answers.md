# Answers

## Question 1

a)

we first access Row 0 Col 0, then Row 1 Col 0 replaces this block the first block, hence Row 0 Col 1 misses

| A     | Col 0 | Col 1 | Col 2 | Col 3 |
| ----- | ----- | ----- | ----- | ----- |
| Row 0 | m     | m     | m     | m     |
| Row 1 | m     | m     | m     | m     |

b)

| A     | Col 0 | Col 1 | Col 2 | Col 3 |
| ----- | ----- | ----- | ----- | ----- |
| Row 0 | m     | h     | m     | h     |
| Row 1 | m     | h     | m     | h     |

## Question 2

### Case 1

a) 100% miss rate, because the blocks with 64 offset conflict with each other

b) 50% miss rate, because every second element is a hit

### Case 2

a) 50%, same as in b) of case 1

b) no, we cannot reduce it any further

c) yes, that can help
