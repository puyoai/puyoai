package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strconv"
	"strings"
)

var (
	featureScoreOnly = flag.Bool("score_only", false, "true if checking the current feature score")
)

func fileExists(filename string) bool {
	if _, err := os.Stat(filename); os.IsNotExist(err) {
		return false
	}

	return true
}

func copyFile(src, dst string) error {
	in, err := os.Open(src)
	if err != nil {
		return err
	}
	defer in.Close()

	out, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer out.Close()

	_, err = io.Copy(out, in)
	cerr := out.Close()
	if err != nil {
		return err
	}

	return cerr
}

func sumMaxScore(filename string) (int, error) {
	buf, err := ioutil.ReadFile(filename)
	if err != nil {
		return 0, err
	}

	sum := 0
	for _, line := range strings.Split(string(buf), "\n") {
		if strings.HasPrefix(line, "max score = ") {
			s := line[len("max score = "):]
			v, err := strconv.Atoi(s)
			if err != nil {
				return 0, err
			}
			sum += v
		}
	}

	return sum, nil
}

func runTweakAndDuel() error {
	err := os.Remove("cpu/mayah/run2.err")
	if err != nil && !os.IsNotExist(err) {
		return err
	}

	if !*featureScoreOnly {
		// Don't tweak if featureScoreOnly.
		cmd := exec.Command("cpu/mayah/tweaker", "--feature=./cpu/mayah/feature.txt")
		if err := cmd.Run(); err != nil {
			return err
		}
	}

	for i := 0; i < 20; i++ {
		fmt.Println("i =", i)
		cmd := exec.Command("./duel/duel", "--num_duel=1", "--realtime=false", "--no_timeout=true",
			fmt.Sprintf("--seed=%d", i), "--use_gui=false", "--use_cui=false", "--httpd=false",
			"--", "cpu/mayah/run_for_tweaker_1p.sh", "cpu/mayah/run_for_tweaker_2p.sh")
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		if err := cmd.Run(); err != nil {
			return err
		}
	}

	return nil
}

func runOnce() error {
	bestScore := 0
	if !*featureScoreOnly && fileExists("cpu/mayah/best_score.txt") && fileExists("cpu/mayah/best_feature.txt") {
		bestScoreBytes, err := ioutil.ReadFile("cpu/mayah/best_score.txt")
		if err != nil {
			return err
		}

		bestScoreStr := strings.TrimRight(string(bestScoreBytes), "\n")
		bestScore, err = strconv.Atoi(bestScoreStr)
		if err != nil {
			return err
		}
		err = copyFile("cpu/mayah/best_feature.txt", "cpu/mayah/feature.txt")
		if err != nil {
			return err
		}
	}

	fmt.Println("previous best score =", bestScore)

	err := runTweakAndDuel()
	if err != nil {
		return err
	}

	score, err := sumMaxScore("cpu/mayah/run2.err")
	if err != nil {
		return err
	}

	fmt.Println("score =", score)

	if !*featureScoreOnly {
		if bestScore < score {
			fmt.Println("beated the previous score. update the feature.")
			if err := ioutil.WriteFile("cpu/mayah/best_score.txt", []byte(fmt.Sprintf("%d", score)), 0600); err != nil {
				return err
			}
			if err := copyFile("cpu/mayah/feature.txt", "cpu/mayah/best_feature.txt"); err != nil {
				return err
			}
		} else {
			if err := copyFile("cpu/mayah/best_feature.txt", "cpu/mayah/feature.txt"); err != nil {
				return err
			}
		}
	}

	return nil
}

func main() {
	flag.Parse()

	fmt.Println("score only :", *featureScoreOnly)

	for {
		if err := runOnce(); err != nil {
			panic(err)
		}

		// Run only once if featureScoreOnly
		if *featureScoreOnly {
			break
		}
	}
}
