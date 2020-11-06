package main

import (
	"database/sql"
	"encoding/hex"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	"github.com/sbinet/go-python"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

// Patch is data structure for a liveupdate patch
type Patch struct {
	Class    string
	Method   string
	FuncName string
	Type     string
	Name     string

	Bytecode []byte
}

// Patches is the global slice of patches. Used primarily for book keeping for -server
var Patches = make([]*Patch, 0)

// DB is the global sql database instance
var DB *sql.DB

// ListAllPatches pretty prints all packages
func ListAllPatches() error{
	files, err := ioutil.ReadDir("patches")
	if err != nil {
		return err
	}

	fmt.Printf(" >> Enabled patches\n")
	for _, v := range files {
		if !v.IsDir() {
			fmt.Printf("%s\n", strings.TrimSuffix(v.Name(), filepath.Ext(v.Name())))
		}
	}

	dfiles, err := ioutil.ReadDir("patches/disabled")
	if err != nil {
		return err
	}

	fmt.Printf(" >> Disabled patches\n")

	for _, v := range dfiles {
		if !v.IsDir() {
			fmt.Printf("%s\n", strings.TrimSuffix(v.Name(), filepath.Ext(v.Name())))
		}
	}
	return nil
}

// EnablePatch moves a patch from patches/disabled to patches
func EnablePatch(name string) error {
	err := os.Rename(fmt.Sprintf("patches/disabled/%s.py", name), fmt.Sprintf("patches/%s.py", name))
	if err != nil {
		return err
	}

	fmt.Printf(" >> Sucessfully enabled %s\n", name)
	
	return nil
}

// DisablePatch moves a patch from patches to patches/disabeld
func DisablePatch(name string) error {
	err := os.MkdirAll("patches/disabled", 0755)
	if err != nil {
		return err
	}

	err = os.Rename(fmt.Sprintf("patches/%s.py", name), fmt.Sprintf("patches/disabled/%s.py", name))
	if err != nil {
		return err
	}

	fmt.Printf(" >> Sucessfully disabled %s\n", name)

	return nil
}

// Patch by name returns a Patch by name from the Global Patches slice
func PatchByName(name string) *Patch {
	for _, v := range Patches {
		if v.Name == name {
			return v
		}
	}
	return nil
}

// DeletePatch deletes a patch from the global Patches slice
func DeletePatch(name string) {
	newPatches := make([]*Patch, 0)
	for _, v := range Patches {
		if v.Name != name {
			newPatches = append(newPatches, v)
		}
	}
	Patches = newPatches
}

// DeletePatchDB will delete a patch from the database by name
func DeletePatchDB(name string) error {
	stm := `
DELETE FROM liveupdates
WHERE updateName=?;
`
	_, err := DB.Exec(stm, name)
	if err != nil {
		return err
	}

	return nil
}

// MakePatches initializes the Patches array with all patches in the patches directory
func MakePatches() {
	err := os.MkdirAll("patches", 0777)
	if err != nil {
		fmt.Printf("[Error] failed to make patches directory %s\n", err.Error())
		return
	}

	patches, err := ioutil.ReadDir("patches")
	if err != nil {
		fmt.Printf("[Error] failed to read patches %s\n", err.Error())
		return
	}

	for _, v := range patches {
		if v.IsDir() {
			continue
		}
		patch, err := MakePatch(v.Name())
		if err != nil {
			fmt.Printf("[Error] %s\n", err.Error())
			return
		}

		Patches = append(Patches, patch)
	}

	fmt.Printf("Found %d patch(es)!\n", len(Patches))
	if !FlagDump {
		fmt.Printf("Not dumping to the database! use -dump\n")
		return
	}
}

// MakePatch creates a *Patch given a filename
func MakePatch(filename string) (*Patch, error) {
	patch := new(Patch)
	data, err := ioutil.ReadFile(filepath.Join("patches", filename))
	if err != nil {
		//fmt.Printf("[Error] Unable to open %s %s\n", v.Name(), err.Error())
		return nil, err
	}
	// Scan for liveupdate meta
	lines := strings.Split(string(data), "\n")
	decorator := ""
	decLine := 0
	for i, line := range lines {
		if len(line) == 0 {
			continue
		}
		fmt.Printf(" > Looking for dectorator in %s\n", filename)
		if (len(line) < 2) {
			continue
		}
		if line[1] == '@' {
			//fmt.Printf("  > Found decorator!\n")
			decLine = i
			decorator = line
			break
		}
	}
	if decorator == "" {
		return patch, fmt.Errorf("failed to find liveupdate decorator")
	}

	argstr := strings.TrimSuffix(strings.Split(decorator, "(")[1], ")")
	args := strings.Split(argstr, ",")
	patch.Name = strings.TrimSuffix(filename, filepath.Ext(filename))
	patch.Type = strings.Trim(args[0], " \"")
	patch.Class = strings.Trim(args[1], " \"")
	patch.Method = strings.Trim(args[2], " \"")
	funcLine := lines[decLine+1]
	patch.FuncName = strings.TrimPrefix(strings.Split(funcLine, "(")[0], "def ")

	fmt.Printf("  > Generated patch metadata! %s %s %s\n", patch.Class, patch.Method, patch.FuncName)

	os.Mkdir("tmp", 0755)
	dumpCode := fmt.Sprintf(`import marshal
file = open('tmp/tmpcode', 'wb')
tmp = marshal.dump(%s.func_code, file)
file.close()
`, patch.FuncName)

	python.PyRun_SimpleString(string(data) + dumpCode)
	patch.Bytecode, err = ioutil.ReadFile("tmp/tmpcode")
	if err != nil {
		return nil, err
	}
	if FlagDebug {
		fmt.Printf("  > Got bytecode for patch! \n\n%s\n\n", hex.EncodeToString(patch.Bytecode))
	}

	fmt.Printf("  > Verifying patch integrity\n")
	pymain := python.PyImport_AddModule("__main__")
	python.PyModule_AddStringConstant(pymain, "s",
		hex.EncodeToString(patch.Bytecode))

	test := "marshal.loads(s.decode('hex'))\n"
	ret := python.PyRun_SimpleString(test)
	if ret != 0 {
		//fmt.Printf("[Error] Python error occured while checking integrity of patch")
		return nil, fmt.Errorf("integrity check failed")
	} else {
		fmt.Printf("    > OK\n")
	}
	return patch, nil
}

// UpdatePatchDB updates a pre-existing patch in the database
func UpdatePatchDB(p *Patch) error {
	stm := `UPDATE liveupdates
SET methodName=?,
    objectID=?,
    codeType=?,
    code=?
WHERE
    updateName=?;
    `

	_, err := DB.Exec(stm, p.Method, p.Class, p.Type, p.Bytecode, p.Name)
	if err != nil {
		return err
	}
	return nil
}

// UpdateDB re-writes the liveupdates table with the global Patches slice
func UpdateDB() error {
	fmt.Printf(" >> Truncating liveupdates\n")
	_, err := DB.Exec("truncate liveupdates;")
	if err != nil {
		fmt.Printf("[Error] Unabled to truncate liveupdates! %s\n", err.Error())
	}
	for i, patch := range Patches {
		// fmt.Printf(`INSERT INTO liveupdates VALUES (%v, %v, %v, %v, %v, %v, %v, %v, %v, %v, %v);`, i, patch.FuncName, "Generated by liveupdategen",
		// 1, 330, 1, 500000, patch.Method, patch.Class, "globalFunction", string(patch.Bytecode))
		stm := `INSERT liveupdates SET
updateID=?, updateName=?, description=?,
machoVersionMin=?, machoVersionMax=?, buildNumberMin=?, buildNumberMax=?,
methodName=?, objectID=?, codeType=?, code=?;`
		_, err := DB.Exec(stm, i, patch.Name, "Generated by liveupdategen",
			1, 330, 1, 500000, patch.Method, patch.Class, patch.Type, patch.Bytecode)
		if err != nil {
			//fmt.Printf("[Error] Failed to insert patch into db! %s\n", err.Error())
			return err
		}

		fmt.Printf(" >> Inserted code into DB for patch %s\n", patch.Name)

		if FlagDebug {
			err := ioutil.WriteFile(filepath.Join("codedump", patch.Name+".raw"), patch.Bytecode, 0777)
			if err != nil {
				//fmt.Printf("Failed to write out codedump file!", err.Error())
				return err
			}
		}
	}
	return nil
}

// UpdatePatch will MakePatch on the filename and either rewrite it in the global patches slice or appends it, additionally returning an instance of the Patch
func UpdatePatch(filename string) (*Patch, error) {
	patch, err := MakePatch(filename)
	if err != nil {
		return nil, err
	}

	for k, v := range Patches {
		if v.Name == patch.Name {
			Patches[k] = patch
			return patch, err
			break
		}
	}
	Patches = append(Patches, patch)
	return patch, nil
}
