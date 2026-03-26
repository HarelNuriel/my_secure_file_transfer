package main

import (
	"bufio"
	"io"
	"os/exec"
	"testing"
)

type dualChannel struct {
	errChan chan error
	strChan chan string
}

func startClientInstance() (*exec.Cmd, io.WriteCloser, io.ReadCloser, error) {
	cmd := exec.Command("/client/msft", "client", "-t", "10.5.0.10")

	stdin, err := cmd.StdinPipe()
	if err != nil {
		return nil, nil, nil, err
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, nil, nil, err
	}

	err = cmd.Start()
	if err != nil {
		return nil, nil, nil, err
	}

	return cmd, stdin, stdout, nil
}

func execRmUsrTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"admin\n", "admin\n", "rm_user test\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "User Removed Successfully.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func newPrivTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"test\n", "test\n", "add_user test2 test2 3\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "User Added Successfully.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func execChmodTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"admin\n", "admin\n", "chmod test 7\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "Changed User Privileges Successfully.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func newUsrTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"test\n", "test\n", "add_user test2 test2 3\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "Insufficient Privileges.\n")
	// outputRoutine(t, reader, '\n', "Could Not Add User.\n")
	outputRoutine(t, reader, '\n', "Could Not Add User.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func execAddUsrTest(t *testing.T, stdin io.WriteCloser, stdout io.ReadCloser) {
	cmd := [...]string{"admin\n", "admin\n", "add_user test test 3\n", "exit\n"}
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
	outputRoutine(t, reader, '\n', "User Added Successfully.\n")
	outputRoutine(t, reader, ' ', "> ")
	sendInput(t, stdin, cmd[3])
}

func TestUsrCmds(t *testing.T) {
	cmd, stdin, stdout, err := startClientInstance()
	if err != nil {
		t.Fatalf("Couldn't start client instance. %s", err)
	}
	execAddUsrTest(t, stdin, stdout)
	err = cmd.Wait()
	if err != nil {
		t.Errorf("Exit Unsuccessful. %s", err)
	}

	cmd, stdin, stdout, err = startClientInstance()
	if err != nil {
		t.Fatalf("Couldn't start client instance 2. %s", err)
	}
	newUsrTest(t, stdin, stdout)
	err = cmd.Wait()
	if err != nil {
		t.Errorf("Exit Unsuccessful. %s", err)
	}

	cmd, stdin, stdout, err = startClientInstance()
	if err != nil {
		t.Fatalf("Couldn't start client instance 3. %s", err)
	}
	execChmodTest(t, stdin, stdout)
	err = cmd.Wait()
	if err != nil {
		t.Errorf("Exit Unsuccessful. %s", err)
	}

	cmd, stdin, stdout, err = startClientInstance()
	if err != nil {
		t.Fatalf("Couldn't start client instance 4. %s", err)
	}
	newPrivTest(t, stdin, stdout)
	err = cmd.Wait()
	if err != nil {
		t.Errorf("Exit Unsuccessful. %s", err)
	}

	cmd, stdin, stdout, err = startClientInstance()
	if err != nil {
		t.Fatalf("Couldn't start client instance 5. %s", err)
	}
	execRmUsrTest(t, stdin, stdout)
	err = cmd.Wait()
	if err != nil {
		t.Fatalf("Exit Unsuccessful. %s", err)
	}

	err = stdin.Close()
	if err != nil {
		t.Logf("Failed to close the stdin pipe. %s", err)
	}

	err = stdout.Close()
	if err != nil {
		t.Logf("Failed to close the stdout pipe. %s", err)
	}
}
