package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"os/exec"
	"slices"
	"sort"
	"testing"
	"time"
)

func validateLs(t *testing.T, reader *bufio.Reader) int {
	f, err := os.Open("/opt/shared/ls_test/")
	if err != nil {
		t.Errorf("Couldn't Open Directory. %s", err)
	}
	defer f.Close()

	files, err := f.Readdir(-1)
	if err != nil {
		t.Errorf("Couldn't Read Directory Files. %s", err)
	}

	outputChannel := dualChannel{errChan: make(chan error), strChan: make(chan string)}
	var recvFiles []string
	var correntFiles []string
	for i := range len(files) {
		go readOutput(reader, '\n', outputChannel)
		select {
		case file := <-outputChannel.strChan:
			correntFiles = append(correntFiles, files[i].Name())
			file = file[:len(file)-1]
			recvFiles = append(recvFiles, file)
		case err := <-outputChannel.errChan:
			t.Errorf("Error Receiving The File's Name. %s", err)
		case <-time.After(5 * time.Second):
			t.Errorf("Timeout.")
		}
	}

	sort.Strings(recvFiles)
	sort.Strings(correntFiles)
	areEqual := slices.Equal(recvFiles, []string(correntFiles))
	if !areEqual {
		t.Errorf("Received Incorrect Files.")
	}

	return len(files)
}

func execLsTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"admin\n", "admin\n", "ls ls_test\n", "exit\n"}
	reader := bufio.NewReader(stdout)

	outputRoutine(t, reader, '\n', "Session started with host: 10.5.0.10\n")
	outputRoutine(t, reader, '\n', "Please Enter Username and Password:\n")
	outputRoutine(t, reader, ' ', "Username: ")
	sendInput(t, stdin, cmd[0])
	outputRoutine(t, reader, ' ', "Password: ")
	sendInput(t, stdin, cmd[1])
	outputRoutine(t, reader, '\n', "Successfully Logged In.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[2])
	files := validateLs(t, reader)
	output := fmt.Sprintf("Listed %d Files.\n", files)
	outputRoutine(t, reader, '\n', "\n")
	outputRoutine(t, reader, '\n', output)
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func TestLsFile(t *testing.T) {
	cmd := exec.Command("/client/msft", "client", "-t", "10.5.0.10")

	stdin, err := cmd.StdinPipe()
	if err != nil {
		t.Errorf("Error Creating stdin Pipe. %s", err)
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		t.Errorf("Error Creating stdout Pipe. %s", err)
	}

	err = cmd.Start()
	if err != nil {
		t.Errorf("Failed To Run The App. %s", err)
	}

	execLsTest(t, stdin, stdout)

	err = stdin.Close()
	if err != nil {
		t.Logf("Failed to close the stdin pipe. %s", err)
	}

	err = stdout.Close()
	if err != nil {
		t.Logf("Failed to close the stdout pipe. %s", err)
	}

	err = cmd.Wait()
	if err != nil {
		t.Errorf("Exit Unsuccessful. %s", err)
	}
}
