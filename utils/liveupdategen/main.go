package main

import (
	"database/sql"
	"flag"
	"fmt"
	"github.com/fsnotify/fsnotify"
	_ "github.com/go-sql-driver/mysql"
	"github.com/sbinet/go-python"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

var (
	// FlagHost is the MySQL host to login to
	FlagHost = "127.0.0.1:3306"
	// FlagUser is the MySQL user to login with
	FlagUser = "root"
	// FlagPassword is the MySQL password to login with
	FlagPassword = ""
	// FlagDBName is the database name to write liveupdates into
	FlagDBName = "evemu"
	// FlagDump will dump all patches into the database.  Additionally it dumps raw code objects for all patches
	FlagDump = false
	// FlagDevTools generates a devtools.raw from devtools.py
	FlagDevTools = false
	// FlagDebug will print debug text to stdout
	FlagDebug = false
	// FlagServer will make liveupdategen run continuously updateing the database as patches are edited in real time
	FlagServer = false
)

func main() {
	flag.StringVar(&FlagHost, "host", "127.0.0.1:3306", "MySQL server host")
	flag.StringVar(&FlagUser, "u", "root", "MySQL username")
	flag.StringVar(&FlagPassword, "p", "", "MySQL password")
	flag.StringVar(&FlagDBName, "db", "evemu", "MySQL database to dump to")
	flag.BoolVar(&FlagDump, "dump", false, "Dump patches to database")
	flag.BoolVar(&FlagDevTools, "dev", false, "Generate devtools.raw")
	flag.BoolVar(&FlagDebug, "debug", false, "Debugging stuff")
	flag.BoolVar(&FlagServer, "server", false, "Run a server to auto-update liveupdates")
	flag.Parse()
	fmt.Println()
	python.Initialize()
	python.PyRun_SimpleString(decos)

	if len(os.Args) > 1 {
		if os.Args[1] == "enable" {
			if len(os.Args) <= 2 {
				fmt.Printf("enable takes one argument")
			}
			err := EnablePatch(os.Args[2])
			if err != nil {
				fmt.Printf("[Error] Failed to enable patch %s\n", err.Error())
			}
			return
		}
		if os.Args[1] == "disable" {
			if len(os.Args) <= 2 {
				fmt.Printf("disable takes one argument")
			}
			err := DisablePatch(os.Args[2])

			if err != nil {
				fmt.Printf("[Error] Failed to disable patch %s\n", err.Error())
			}
			return
		}
		if os.Args[1] == "list" {
			err := ListAllPatches()
			if err != nil {
				fmt.Printf("[Error] Failed to list patches %s\n", err.Error())
			}
			return
		}
	}

	if FlagDebug {
		os.Mkdir("codedump", 0777)
	}

	if FlagDevTools {
		fmt.Printf(" >> Generating devtools.raw\n")
		_, err := ioutil.ReadFile("devtools.py")
		if err != nil {
			fmt.Printf("[Error] Failed to open devtools.py\n")
			return
		}
		MakeDevTools()
		return
	}

	var err error

	DB, err = sql.Open("mysql", fmt.Sprintf("%s:%s@tcp(%s)/%s", FlagUser, FlagPassword, FlagHost, FlagDBName))
	if err != nil {
		fmt.Printf("[Error] Failed connect to the database! %s\n", err.Error())
		return
	}
	err = DB.Ping()
	if err != nil {
		fmt.Printf("[Error] DB Ping failed! %s", err.Error())
		return
	}

	if FlagDump {
		MakePatches()
		UpdateDB()
		return
	}

	if FlagServer {
		MakePatches()
		fmt.Printf(" >> Starting server mode\n")
		done := make(chan struct{})
		watcher, err := fsnotify.NewWatcher()
		if err != nil {
			fmt.Printf("[Error] Failed to create a fsnotify watcher %s\n", err.Error())
			return
		}
		go func() {
			for {
				select {
				case event := <-watcher.Events:
					fmt.Printf(" >> New FS event: %v\n", event)
					_, f := filepath.Split(event.Name)
					name := strings.TrimSuffix(f, filepath.Ext(f))
					if event.Op&fsnotify.Write == fsnotify.Write {
						if filepath.Ext(f) == ".py" &&
							f[0] != '.' {
							fmt.Printf("  > Modified patch file: %s\n", event.Name)
							p, err := UpdatePatch(f)

							if err != nil {
								fmt.Printf("[Error] Failed to update patch in place: %s\n", err.Error())
							}
							err = UpdatePatchDB(p)
							if err != nil {
								fmt.Printf("[Error] Failed to update DB for in place patching: %s\n", err.Error())
							}
						}
					}
					if event.Op&fsnotify.Remove == fsnotify.Remove {
						DeletePatch(name)
						err := DeletePatchDB(name)
						if err != nil {
							fmt.Printf("[Error] failed to delete patch from db: %s\n", err.Error())
						}
					}
				case err := <-watcher.Errors:
					fmt.Printf(" >> FS error: %s\n", err.Error())
				}
			}
		}()

		err = watcher.Add("patches/")
		if err != nil {
			fmt.Printf("[Error] Failed to add watcher for patches/ %s\n", err.Error())
		}

		fmt.Printf(" >> Checking patch updates in patches/\n")
		<-done
	}
}
