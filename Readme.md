## Photo Alignment
Transforms a batch of slightly tranposed/rotated input images so that the features of the images overlap.

For example, a batch of moon photos taken over a period of time (e.g. 1min).
The moon will move on the photos.
This program transforms each photo such that the the moon is on the same position.

The output images are .png files with enabled alpha channel.
Every coordinate without image information through the transformation has the color value 0.

Windows pre-build binary can be found [here](https://github.com/TinyTinni/PhotoAlignment/releases/).

## Usage
Use the console to call the program.
```
  ./PhotoAlignment [<reference image>] options

where options are:
  -?, -h, --help                         display usage information
  -i, --input-dir <input directory>      Input directory containing all
                                         images which should be transformed.
  -o, --output-dir <output directory>    Output directory where the
                                         transformed images are written into
                                         (overwrites images with the same
                                         name).
```

Example:
```
PhotoAlignment mid_image.jpg -i ./my_image_series -o ./transformed
```

## Build Requirements
- [OpenCV](https://opencv.org/)
- C++14 (with experimental filesystem support)
- OpenMP (optional)

## Additional use of the following libs:
- [spdlog](https://github.com/gabime/spdlog)
- [clara](https://github.com/catchorg/Clara)

## License
[BSD3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
