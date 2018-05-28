//
// Created by raphael on 07.05.18.
//

#include "ImageGenerator.h"

#include <climits>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <spdlog/spdlog.h>

std::vector<unsigned char> ImageGenerator::marble(unsigned int width, unsigned int height, glm::vec4 color, bool rgba) {
    std::vector<unsigned char> data;
    data.reserve(height * width * (rgba ? 4 : 3));

    for (int x=0; x < width; x++)
        for (int y=0; y < height; y++) {
            float nx = (float)x / width - 0.5f;
            float ny = (float)y / height - 0.5f;
            float nz = 15.296f;


            auto noise =
                    1    * stb_perlin_ridge_noise3(2 * nx, 2 * ny, nz, 2.0, 0.5, 1.0, 24, 256, 256, 64) +
                    .5   * stb_perlin_ridge_noise3(4 * nx, 4 * ny, nz, 2.0, 0.5, 1.0, 24, 128, 128, 64) +
                    .25  * stb_perlin_ridge_noise3(8 * nx, 4 * ny, nz, 2.0, 0.5, 1.0, 24, 64, 64, 32);
                    //.125 * stb_perlin_ridge_noise3(25 * nx, 27 * ny, nz, 2.0, 0.5, 1.0, 24, 256, 256, 64);
            auto value = glm::pow(noise / 2.0f, 0.68298504);

            //spdlog::get("console")->info("({},{}) noise: {}, value: {}", x,y, noise, value);

            data.push_back(static_cast<unsigned char &&>(value * color.x * UCHAR_MAX));
            data.push_back(static_cast<unsigned char &&>(value * color.y * UCHAR_MAX));
            data.push_back(static_cast<unsigned char &&>(value * color.z * UCHAR_MAX));

            if (rgba) data.push_back(static_cast<unsigned char &&>(color.w * UCHAR_MAX));
        }

    return data;
}
