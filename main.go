package main

import (
	"fmt"
	"github.com/rwcarlsen/goexif/exif"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strconv"
)

var dirname = "/temp"
var dirname2 = "/temp2" + string(filepath.Separator)
var months = make(map[string]string)

func sortImage(file os.FileInfo) (error, string) {
	filePath := path.Join(dirname, file.Name())
	f, err := os.Open(filePath)

	if err != nil {
		_ = f.Close()
		return err, ""
	}

	x, err := exif.Decode(f)
	if err != nil {
		return err, ""
	}
	_ = f.Close()

	tm, _ := x.DateTime()
	day := fmt.Sprintf("%02d", tm.Day())
	month := months[tm.Month().String()]
	year := strconv.Itoa(tm.Year())

	exportPath := dirname2 + year + string(filepath.Separator) + month + string(filepath.Separator) + day
	_ = os.MkdirAll(exportPath, 0777)

	dst := path.Join(exportPath, file.Name())
	err = os.Rename(filePath, dst)
	if err != nil {
		return err, ""
	}
	return nil, dst
}

func init() {
	months["January"] = "01"
	months["February"] = "02"
	months["March"] = "03"
	months["April"] = "04"
	months["May"] = "05"
	months["June"] = "06"
	months["July"] = "07"
	months["August"] = "08"
	months["September"] = "09"
	months["October"] = "10"
	months["November"] = "11"
	months["December"] = "12"
}
func showHappyEnd() {
	fmt.Println()
	fmt.Println("/   \\          /   \\\n\\_   \\        /  __/\n _\\   \\      /  /__\n \\___  \\____/   __/\n     \\_       _/\n       | @ @  \\_\n       |\n     _/     /\\\n    /o)  (o/\\ \\_\n    \\_____/ /\n      \\____/")
}

func main() {
	files, err := ioutil.ReadDir(dirname)
	if err != nil {
		panic(err)
	}
	numberFiles := len(files)

	i := 0
	for _, file := range files {
		i++
		fmt.Print(fmt.Sprintf("%3d", int(float64(i)/float64(numberFiles)*100)) + "%| " + file.Name())

		err, dstFilePath := sortImage(file)
		if err != nil {
			fmt.Print("\n")
			fmt.Println(err)
			continue
		}
		fmt.Print(" to " + dstFilePath + "\n")
	}

	showHappyEnd()
}
