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

#include <boinc_api.h>
#include <filesys.h>

// stage is 0 during chaos, 1 during density estimation.
// percent restarts during each stage.
int myprogress(void* param, double percent, int stage, double eta) {
    if (stage == 0) {
        boinc_fraction_done(percent/100.0);
    }
    return 0;
}

int main()
{
    boinc_init();

    flam3_frame f;

    memset(&f, 0, sizeof(f));
    flam3_init_frame(&f);
    flam3_srandom();

    char real_in_fn[512];
    boinc_resolve_filename("in.flam3", real_in_fn, sizeof(real_in_fn));
    FILE* fp = boinc_fopen(real_in_fn, "rb");

    flam3_genome* cps;
    int ncps;
    cps = flam3_parse_from_file(fp, real_in_fn, flam3_defaults_on, &ncps);
    if (!cps) {
        fprintf(stderr, "Can't parse file\n");
        boinc_finish(1);
    }
    fclose(fp);

    if (ncps > 1) {
        fprintf(stderr, "This program only supports a single flame per input file\n");
        boinc_finish(1);
    }

    cps[0].ntemporal_samples = 1;

    f.genomes = cps;
    f.ngenomes = 1;
    f.verbose = 0;
    f.nthreads = 1;
    f.bytes_per_channel = 1;
    f.bits = 33; // 33 is double-precision floating point
    f.pixel_aspect_ratio = 1.0;
    f.progress = myprogress;
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
    char real_out_fn[512];
    boinc_resolve_filename("out.png", real_out_fn, sizeof(real_out_fn));
    FILE* outfp = boinc_fopen(real_out_fn, "wb");
    write_png(outfp, image_data.get(), cps[0].width, cps[0].height, nullptr, 1);
    fclose(outfp);

    free(cps);

    boinc_finish(0);
}
