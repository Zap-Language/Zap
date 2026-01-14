func quicksort([]int arr, int left, int right) {
    if (left + 1 > right) {
        return
    }

    let i = left
    let j = right
    let pivot = arr[(left + right) / 2]

    while (i < j + 1) {
        while (arr[i] < pivot) {
            i = i + 1
        }
        while (arr[j] > pivot) {
            j = j - 1
        }

        if (i < j + 1) {
            let tmp = arr[i]
            arr[i] = arr[j]
            arr[j] = tmp

            i = i + 1
            j = j - 1
        }
    }

    if (left < j) {
        quicksort(arr, left, j)
    }
    if (i < right) {
        quicksort(arr, i, right)
    }
}

let arr = []int(int(read()))

for (let i = 0; i < len(arr); i = i + 1) {
    arr[i] = int(read())
}

quicksort(arr, 0, len(arr) - 1)
print(arr)