func bubbleSort([]int arr) []int {
    let n = len(arr)
    let i = 0
    while (i < n) {
        let j = 0
        while (j + 1 < n - i) {
            if (arr[j] > arr[j + 1]) {
                let tmp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = tmp
            }
            j = j + 1
        }
        i = i + 1
    }
    return arr
}

let data = []int{5, 1, 4, 2, 8}
let sorted = bubbleSort(data)
print(sorted)