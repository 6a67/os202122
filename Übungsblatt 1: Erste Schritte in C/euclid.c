int euclidean_gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return euclidean_gcd(b, a % b);
}