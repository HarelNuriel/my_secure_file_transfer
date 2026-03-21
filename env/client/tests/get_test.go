package main

import (
	"bufio"
	"io"
	"os/exec"
	"testing"
	"time"
)

type dualChannel struct {
	errChan chan error
	strChan chan string
}

func readOutput(reader *bufio.Reader, delim byte, channel dualChannel) {
	clientLog, err := reader.ReadString(delim)
	if err != nil {
		channel.errChan <- err
		return
	}

	channel.strChan <- clientLog
}

func outputRoutine(t *testing.T, reader *bufio.Reader, delim byte, match string) {
	outputChannel := dualChannel{errChan: make(chan error), strChan: make(chan string)}
	go readOutput(reader, delim, outputChannel)

	select {
	case <-time.After(5 * time.Second):
		t.Fatalf("Timeout wait for output.")
	case clientLog := <-outputChannel.strChan:
		if clientLog != match {
			t.Fatalf("Output Mismatch. %s != %s", clientLog, match)
		}
	case err := <-outputChannel.errChan:
		t.Fatalf("Error Receiving Client Output. %s", err)
	}
}

func sendInput(t *testing.T, stdin io.WriteCloser, message string) {
	_, err := io.WriteString(stdin, message)
	if err != nil {
		t.Fatalf("Couldn't send input via stdin. %s", err)
	}
}

func execTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"admin\n", "admin\n", "get pic.jpeg\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "Writing To: pic.jpeg.\n")
	outputRoutine(t, reader, '\n', "File Received Successfully.\n")
	outputRoutine(t, reader, '\n', "Successfully Received pic.jpeg.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func TestGetFile(t *testing.T) {
	cmd := exec.Command("/client/msft", "client", "-t", "10.5.0.10")

	stdin, err := cmd.StdinPipe()
	if err != nil {
		t.Fatalf("Error Creating stdin Pipe. %s", err)
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		t.Fatalf("Error Creating stdout Pipe. %s", err)
	}

	err = cmd.Start()
	if err != nil {
		t.Fatalf("Failed To Run The App. %s", err)
	}

	execTest(t, stdin, stdout)

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
