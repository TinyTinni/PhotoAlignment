
#include <opencv/cv.hpp>

#include <clara.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <iostream>

#include <filesystem>
#include <string>
#include <vector>
#include <numeric>

#include <execution>
#include <atomic>
#include <chrono>

namespace fs = std::experimental::filesystem;

std::shared_ptr<spdlog::logger> g_log = nullptr;

struct CmdLineOptions
{
    fs::path ref_image = "";
    fs::path input_dir = "./";
    fs::path output_dir = "./";
};

/** parse command line. Exits the program on error
    !! also initialized global variable log  !!
*/
CmdLineOptions parse_command_line(int argc, char* argv[])
{
    CmdLineOptions result;

    bool showHelp = false;
    auto cli = clara::Help(showHelp)
        | clara::Opt(result.input_dir, "input directory")
        ["-i"]["--input-dir"]("Input directory containing all images which should be transformed.")
        | clara::Opt(result.output_dir, "output directory")
        ["-o"]["--output-dir"]("Output directory where the transformed images are written into (overwrites images with the same name).")
        | clara::Arg(result.ref_image, "reference image")
        ("Reference image which defines the feature position for the output images.")
        ;

    {
        const auto r = cli.parse(clara::Args(argc, argv));
        if (!r)
        {
            std::cerr << "Error in command line: " << r.errorMessage() << std::endl;
            exit(1);
        }
        if (showHelp || result.ref_image.empty())
        {
            std::cout << R"(
Transforms given images in input-dir in such a way 
that the found features
are on the same position as the found
features in [<reference image>].
All files in output dir with the same name will be overwritten.
)";
            std::cout << cli << std::endl;
            exit(1);
        }
    }

    try {
        g_log = spdlog::stdout_color_mt("main");
    }
    catch (const spdlog::spdlog_ex& e)
    {
        std::cerr << "Cannot create logger: " << e.what() << std::endl;
        exit(1);
    }

    if (!fs::is_regular_file(result.ref_image))
    {
        g_log->critical("Reference image does not exist: {}", result.ref_image);
        exit(1);
    }

    if (!fs::is_directory(result.input_dir))
    {
        g_log->critical("Input Directory is not a dir or does not exist: {}", result.input_dir);
        exit(1);
    }

    if (!fs::exists(result.output_dir))
    {
        g_log->warn("Output Directory does not exist. Create new one.");
        fs::create_directory(result.output_dir);
    }

    return result;
}

template<typename T>
void transform_files(const cv::Mat ref_img, std::vector<fs::path> files , const fs::path& output_dir, const T& callable)
{
    const size_t num_files = files.size();
    std::mutex output_mutex;
    size_t processed = 0;

    std::for_each(std::execution::par, std::begin(files), std::end(files),
        [&](const auto& file)
    {
        const auto input_str = file.string();
        auto output_file = output_dir;
        output_file /= file.filename();
        output_file.replace_extension(".png");
        const auto output_str = output_file.string();

        cv::Mat img = cv::imread(input_str, cv::ImreadModes::IMREAD_UNCHANGED);

        if (!img.data)
        {
            g_log->error("Could not read: {} \nSkipping it.", input_str);
        }
        else
        {
            try
            {
                cv::Mat transformed;
                cv::Mat M = cv::estimateRigidTransform(img, ref_img, false);
                //lancros interpolation since we can expect rotations

                if (img.channels() != 4)
                {
                    // add alpha channel
                    cv::Mat ones = cv::Mat::ones(img.size().height, img.size().width, CV_8UC1);
                    ones *= 255;
                    std::vector<cv::Mat> v{ std::move(img), std::move(ones) };
                    cv::merge(std::move(v), img);
                }
                cv::warpAffine(img, transformed, M, img.size(), cv::INTER_LANCZOS4, cv::BORDER_TRANSPARENT);
                cv::imwrite(output_str, transformed);
            }
            catch (const std::exception &e)
            {
                g_log->error("Error while processing \"{}\" - {}", input_str, e.what());
            }
        }
        const size_t p = ++processed;
        std::lock_guard<std::mutex> lm(output_mutex);
        fmt::print("\rProcessed files {}/{}", p, num_files);
    });
    fmt::print("\n");
}

int main(int argc, char* argv[])
{
    spdlog::set_pattern("[%t] %L: %v");
    const auto options = parse_command_line(argc, argv);
    
    const cv::Mat ref_img = cv::imread(options.ref_image.string(), cv::ImreadModes::IMREAD_UNCHANGED);
    if (!ref_img.data)
    {
        g_log->critical("Could not read reference image: {}", options.ref_image);
        return 1;
    }

    const std::vector<fs::path> files(fs::begin(fs::directory_iterator(options.input_dir)),
        fs::end(fs::directory_iterator(options.input_dir)));

    fmt::print("Ref Image: {}\nInput Dir: {}\n", options.ref_image, options.input_dir);

    const auto start_time = std::chrono::high_resolution_clock::now();
    transform_files(ref_img, files, options.output_dir, []() {});    
    float time_sec = 0.001f*std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
    
    fmt::print("Done in {0:.3f}s\n", time_sec);

    return 0;
}
