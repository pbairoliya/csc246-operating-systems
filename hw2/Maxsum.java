
/**
 * @file Maxsum.java  maxsum program that finds the longest contiguous maximum sum of an array using threads.
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * @date 2022-09-29
 * 
 * CSC 246 HW2
 * 
 */

import java.util.ArrayList;
import java.util.Scanner;

/**
 * This is the Maxsum class that has a list in which contains a public ArrayList
 * full of all the integers in the input file
 * This class also has a thread that extends the thread class which allows to be
 * run multiple times.
 */
public class Maxsum {
    public static ArrayList<Integer> list = new ArrayList<Integer>();

    static class MyThread extends Thread {

        public int maxsum = 0;
        public int threadNumber = 0;
        public int numWorkers = 0;

        /** Make a new Thread, giving it a parameter value to store. */
        public MyThread(int n, int numWorkers) {
            this.threadNumber = n;
            this.maxsum = 0;
            this.numWorkers = numWorkers;
        }

        // the running method of the thread
        public void run() {
            // the current Max of one for loop iteration
            this.maxsum = 0;
            int curr_max = 0;
            for (int j = this.threadNumber; j < list.size(); j += numWorkers) {
                curr_max = 0;
                for (int k = j; k < list.size(); k++) {
                    curr_max += list.get(k);
                    if (this.maxsum <= curr_max) {
                        this.maxsum = curr_max;
                    }
                }

            }
        }
    }

    // Print out a usage message, then exit.
    public static void usage() {
        System.out.println("usage: Maxsum <workers>");
        System.out.println("       maxsum <workers> report");
        System.exit(1);
    }

    // Print out an error message and exit.
    public static void fail(String message) {
        System.err.println(message);
        System.exit(1);
    }

    public static void readList() {
        Scanner in = new Scanner(System.in);
        while (in.hasNextInt()) {
            list.add(in.nextInt());
        }
        in.close();
    }

    public static void main(String[] args) {

        Boolean report = false;

        if (args.length < 1 || args.length > 2) {

            usage();
        }

        if (args[0].isEmpty() || Integer.parseInt(args[0]) < 1) {
            usage();
        }
        int numWorkers = Integer.parseInt(args[0]);

        if (args.length == 2) {
            if (!args[1].equals("report")) {
                usage();
            }
            report = true;
        }
        readList();

        MyThread[] thread = new MyThread[numWorkers];

        for (int i = 0; i < thread.length; i++) {
            thread[i] = new MyThread(i, numWorkers);
            thread[i].start();
        }
        int actualMaxSum = 0;

        // Wait for each of the threads to terminate.
        try {
            for (int i = 0; i < thread.length; i++) {

                thread[i].join();
                if (report) {
                    System.out.println("I'm thread " + thread[i].getId() + ". The maximum sum I found is "
                            + thread[i].maxsum + ".");
                }
                if (actualMaxSum <= thread[i].maxsum) {
                    actualMaxSum = thread[i].maxsum;
                }
            }
            System.out.println("Maximum sum: " + actualMaxSum + ".");
        } catch (InterruptedException e) {
            System.out.println("Interrupted during join!");
        }
    }
}