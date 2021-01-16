package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/rwcarlsen/goexif/exif"
	"io"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"
	"trimmer.io/go-xmp/models/xmp_base"
	"trimmer.io/go-xmp/xmp"
)

var sourceDir = string(filepath.Separator) + "temp"
var exportDir = string(filepath.Separator) + "temp2"

var months = make(map[string]string)

const appleEpochAdjustment = 2082844800

const (
	movieResourceAtomType   = "moov"
	movieHeaderAtomType     = "mvhd"
	referenceMovieAtomType  = "rmra"
	compressedMovieAtomType = "cmov"
)

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

func main() {
	files, err := ioutil.ReadDir(sourceDir)
	if err != nil {
		panic(err)
	}
	numberFiles := len(files)

	for i, file := range files {
		fmt.Print(fmt.Sprintf("%3d", int(float64(i)/float64(numberFiles)*100)) + "%| " + file.Name())

		filePath := path.Join(sourceDir, file.Name())
		fd, err := os.Open(filePath)
		if err != nil {
			printError(err)
			continue
		}

		var created time.Time

		switch strings.ToLower(filepath.Ext(filePath)) {
		case ".mov":
			created, err = getVideoCreationTimeMetadata(fd)
			break
		case ".xmp":
			created, err = getXmpCreationTimeMetadata(fd)
			break
		default:
			created, err = sortImage(fd)
		}

		if err != nil {
			printError(err)
			err = fd.Close()
			if err != nil {
				printError(err)
			}
			continue
		}

		err = fd.Close()
		if err != nil {
			printError(err)
			continue
		}

		dstFilePath, err := moveFileToNewLocation(filePath, file.Name(), created)
		if err != nil {
			printError(err)
			continue
		}

		fmt.Print(" to " + dstFilePath + "\n")
	}

	showHappyEnd()
}

func getVideoCreationTimeMetadata(videoBuffer io.ReadSeeker) (time.Time, error) {
	buf := make([]byte, 8)

	// Traverse videoBuffer to find movieResourceAtom
	for {
		// bytes 1-4 is atom size, 5-8 is type
		// Read atom
		if _, err := videoBuffer.Read(buf); err != nil {
			return time.Time{}, err
		}

		if bytes.Equal(buf[4:8], []byte(movieResourceAtomType)) {
			break // found it!
		}

		atomSize := binary.BigEndian.Uint32(buf)         // check size of atom
		_, err := videoBuffer.Seek(int64(atomSize)-8, 1) // jump over data and set seeker at beginning of next atom
		if err != nil {
			return time.Time{}, err
		}
	}

	// read next atom
	if _, err := videoBuffer.Read(buf); err != nil {
		return time.Time{}, err
	}

	atomType := string(buf[4:8]) // skip size and read type
	switch atomType {
	case movieHeaderAtomType:
		// read next atom
		if _, err := videoBuffer.Read(buf); err != nil {
			return time.Time{}, err
		}

		// byte 1 is version, byte 2-4 is flags, 5-8 Creation time
		appleEpoch := int64(binary.BigEndian.Uint32(buf[4:])) // Read creation time

		return time.Unix(appleEpoch-appleEpochAdjustment, 0).Local(), nil
	case compressedMovieAtomType:
		return time.Time{}, errors.New("compressed video")
	case referenceMovieAtomType:
		return time.Time{}, errors.New("reference video")
	default:
		return time.Time{}, errors.New("did not find movie header atom (mvhd)")
	}
}

func getXmpCreationTimeMetadata(f io.ReadSeeker) (time.Time, error) {
	b, err := ioutil.ReadAll(f)
	if err != nil {
		return time.Time{}, err
	}
	d := &xmp.Document{}
	if err := xmp.Unmarshal(b, d); err != nil {
		return time.Time{}, err
	}
	model := xmpbase.FindModel(d)
	if model == nil {
		return time.Time{}, errors.New("can't get xmp model")
	}
	return time.Time(model.CreateDate), nil
}

func sortImage(f io.ReadSeeker) (time.Time, error) {
	resultExif, err := exif.Decode(f)
	if err != nil {
		return time.Time{}, err
	}

	resultDateTime, err := resultExif.DateTime()
	if err != nil {
		return time.Time{}, err
	}

	return resultDateTime, nil
}

func moveFileToNewLocation(filePath string, fileName string, created time.Time) (string, error) {
	day := fmt.Sprintf("%02d", created.Day())
	month := months[created.Month().String()]
	year := strconv.Itoa(created.Year())

	exportPath := path.Join(exportDir, year, month, day)
	err := os.MkdirAll(exportPath, 0777)
	if err != nil {
		return "", err
	}

	dst := path.Join(exportPath, fileName)
	err = os.Rename(filePath, dst)
	if err != nil {
		return "", err
	}
	return dst, nil
}

func printError(err error) {
	fmt.Print("\n")
	fmt.Println(err)
}

func showHappyEnd() {
	fmt.Println()
	fmt.Println("/   \\          /   \\\n\\_   \\        /  __/\n _\\   \\      /  /__\n \\___  \\____/   __/\n     \\_       _/\n       | @ @  \\_\n       |\n     _/     /\\\n    /o)  (o/\\ \\_\n    \\_____/ /\n      \\____/")
}
