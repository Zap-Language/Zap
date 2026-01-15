func sieve(int n) []bool {
    let primes = []bool(n + 1)
    for (let i = 0; i < n + 1; i = i + 1) {
	    primes[i] = true
    }

    for (let i = 2; i < n + 1; i = i + 1) {
    	if (primes[i]) {
    	    for (let j = 2 * i; j < n + 1; j = j + i) {
    		    primes[j] = false
    	    }
    	}
    }

    return primes
}

let sieved = sieve(int(read()))

for (let i = 1; i < len(sieved); i = i + 1) {
    if (sieved[i]) {
        print(i)
    }
}