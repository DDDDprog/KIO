public class SimpleBenchmarkJava {
    public static void main(String[] args) {
        final int total = 1_000_000;
        long sum = 0L;

        long start = System.nanoTime();
        for (int i = 0; i < total; i++) {
            sum += i;
        }
        long end = System.nanoTime();

        double ms = (end - start) / 1_000_000.0;

        System.out.println("=== Java Simple Benchmark ===");
        System.out.println("Sum: " + sum);
        System.out.println("Time: " + ms + " ms");
    }
}

