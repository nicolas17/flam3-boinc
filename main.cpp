/*
    FLAM3-BOINC
    Copyright (C) 2015 Nicolás Alvarez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

extern "C" {
#include "flam3.h"
#include "img.h"
}

int myprogress(void* param, double percent, int unknown, double eta) {
    //printf();
    return 0;
}

int main()
{
    flam3_frame f;

    memset(&f, 0, sizeof(f));
    flam3_init_frame(&f);
    flam3_srandom();

    const char* filename = "in.flam3";
    FILE* fp = fopen(filename, "rb");

    flam3_genome* cps;
    int ncps;
    cps = flam3_parse_from_file(fp, filename, flam3_defaults_on, &ncps);
    if (!cps) {
        fprintf(stderr, "Can't parse file\n");
        exit(1);
    }
    fclose(fp);

    if (ncps > 1) {
        fprintf(stderr, "This program only supports a single flame per input file\n");
        exit(1);
    }

    cps[0].ntemporal_samples = 1;

    f.genomes = cps;
    f.ngenomes = 1;
    f.verbose=1;
    f.nthreads = 1;
    f.bytes_per_channel = 1;
    f.bits = 33; // 33 is double-precision floating point
    f.pixel_aspect_ratio = 1.0;
    //f.progress = myprogress;
    f.sub_batch_size = 10000;

    const int channels = 4;

    size_t needed_memory = cps[0].width * cps[0].height * channels /* * bytes_per_channel */;

    std::unique_ptr<char[]> image_data(new char[needed_memory]);
    std::fill(image_data.get(), image_data.get()+needed_memory, 0);

    stat_struct stats;
    if (flam3_render(&f, image_data.get(), flam3_field_both, channels, 0, &stats) != 0) {
        fprintf(stderr, "error rendering image: aborting.\n");
        exit(1);
    }

    putenv("enable_png_comments=0");
    FILE* outfp = fopen("out.png", "wb");
    write_png(outfp, image_data.get(), cps[0].width, cps[0].height, nullptr, 1);
    fclose(outfp);

    free(cps);
}
