package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/rwcarlsen/goexif/exif"
	xmpbase "trimmer.io/go-xmp/models/xmp_base"
	"trimmer.io/go-xmp/xmp"
)

type MediaFileType int

const (
	ImageType MediaFileType = iota
	VideoType
	XMPType
	UnknownMediaType
)

type FileInfoStruct struct {
	absolutePath string
	fileInfo     fs.FileInfo
}

var sourceDir = string(filepath.Separator) + "temp"
var exportDir = string(filepath.Separator) + "temp2"
var videoExportDir = string(filepath.Separator) + "tempVideo"
var months = make(map[string]string)
var useModificationTimeAsCreated = bool(false)
var syncSTDInOut = bool(false)
var filesList []FileInfoStruct

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
	flag.StringVar(&sourceDir, "source_dir", string(filepath.Separator)+"temp", "Source directory")
	flag.StringVar(&exportDir, "export_dir", string(filepath.Separator)+"temp2", "Export directory")
	flag.StringVar(&videoExportDir, "video_export_dir", string(filepath.Separator)+"tempVideo", "Video export directory")
	flag.BoolVar(&useModificationTimeAsCreated, "use_mod_time_as_created", false, "Use modification time as created when no date in exif or filename")
	flag.BoolVar(&syncSTDInOut, "sync_std_in_out", false, "Sync STD In Out")
	flag.Parse()

	err := parseDirectory(sourceDir)
	if err != nil {
		printError(err)
		return
	}

	scanner := bufio.NewScanner(os.Stdin)
	writer := bufio.NewWriter(os.Stdout)

	if len(filesList) > 0 {
		numberFiles := len(filesList)
		for i, file := range filesList {
			fmt.Fprint(writer, fmt.Sprintf("%3d", int(float64(i)/float64(numberFiles)*100))+"%| "+file.fileInfo.Name())
			dstFilePath, err := parseFile(file)
			if err != nil {
				fmt.Fprintln(writer, " "+err.Error())
				continue
			}
			fmt.Fprint(writer, " to "+dstFilePath+"\n")
			writer.Flush()
			if syncSTDInOut {
				scanner.Scan()
			}
		}
	}

	showHappyEnd()
}

func parseFile(file FileInfoStruct) (string, error) {
	fd, err := os.Open(file.absolutePath)
	if err != nil {
		return "", err
	}

	var created time.Time

	var fileType MediaFileType
	switch strings.ToLower(filepath.Ext(file.absolutePath)) {
	case ".mov":
		created, err = getVideoCreationTimeMetadata(fd)
		fileType = VideoType
	case ".mp4":
		created, err = getVideoCreationTimeMetadata(fd)
		fileType = VideoType
	case ".avi":
		created, err = getVideoCreationTimeMetadata(fd)
		fileType = VideoType
	case ".3gp":
		created, err = getVideoCreationTimeMetadata(fd)
		fileType = VideoType
	case ".thm":
		created, err = sortImage(fd)
		fileType = VideoType
	case ".xmp":
		created, err = getXmpCreationTimeMetadata(fd)
		fileType = XMPType
	default:
		created, err = sortImage(fd)
		fileType = ImageType
	}

	fd.Close()

	if err != nil {
		//Порпробуем выдрать дату из файла
		re, _ := regexp.Compile(`\d{8}`)
		res := re.FindString(file.fileInfo.Name())
		if len(res) != 0 {
			year, _ := strconv.ParseInt(res[0:4], 10, 64)
			month, _ := strconv.ParseInt(res[4:6], 10, 64)
			day, _ := strconv.ParseInt(res[6:8], 10, 64)

			created = time.Date(int(year), time.Month(month), int(day), 0, 0, 0, 0, time.UTC)
		} else {
			if !useModificationTimeAsCreated {
				return "", err
			}

			info, _ := os.Stat(file.absolutePath)
			created = info.ModTime()
		}
	}

	dstFilePath, err := moveFileToNewLocation(file.absolutePath, file.fileInfo.Name(), fileType, created)
	if err != nil {
		return "", err
	}

	return dstFilePath, nil
}

func parseDirectory(dirpath string) error {
	files, err := os.ReadDir(dirpath)
	if err != nil {
		return err
	}

	for _, file := range files {
		if file.IsDir() {
			err = parseDirectory(dirpath + string(filepath.Separator) + file.Name())
			if err != nil {
				return err
			}
			continue
		}
		fileInfo, _ := file.Info()
		filesList = append(filesList, FileInfoStruct{dirpath + string(filepath.Separator) + fileInfo.Name(), fileInfo})
	}

	return nil
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
	b, err := io.ReadAll(f)
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

func moveFileToNewLocation(filePath string, fileName string, fileType MediaFileType, created time.Time) (string, error) {
	day := fmt.Sprintf("%02d", created.Day())
	month := months[created.Month().String()]
	year := strconv.Itoa(created.Year())

	var exportPath string

	switch fileType {
	case ImageType:
		exportPath = path.Join(exportDir, year, month, day)
	case XMPType:
		exportPath = path.Join(exportDir, year, month, day)
	case VideoType:
		exportPath = path.Join(videoExportDir, year, month, day)
	}

	err := os.MkdirAll(exportPath, 0777)
	if err != nil {
		return "", err
	}

	dst := path.Join(exportPath, fileName)

	if _, err := os.Stat(dst); err == nil {
		return dst, errors.New("file already exist")
	} else if errors.Is(err, os.ErrNotExist) {
		// ok
	} else {
		return dst, errors.New("some problems")
	}

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
