func fibonacci(int n) int {
    if (n < 2) {
        return n
    }
    let a = fibonacci(n - 1)
    let b = fibonacci(n - 2)
    return a + b
}

let result = fibonacci(21)
print(result)