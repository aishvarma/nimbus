/*
 * Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Chinmayee Shah <chinmayee.shah@stanford.edu>
 */

#include "lib/nimbus.h"
#include "./water_app.h"

void WaterApp::load() {

    /* Declare and initialize data and jobs. */
    std::cout << "Worker beginning to load application" << std::endl;

    registerJob("main", new Main(this, COMP));

    registerJob("init", new Init(this, COMP));
    registerJob("loop", new Loop(this, COMP));

    registerData("mac_grid_1", new Grid<TV>);
    registerData("mpi_grid_1", new MPIGrid<TV>);
    registerData("face_velocities_1", new FaceArray<TV>);
    registerData("face_velocities_ghost_1", new FaceArrayGhost<TV>);
    registerData("sim_data_1", new NonAdvData<TV, T>);

    std::cout << "Finished creating job and data definitions" << std::endl;

    std::cout << "Finished loading application" << std::endl;
}

Main::Main(WaterApp *app, JobType type)
    : Job(app, type)
{
};

void Main::Execute(std::string params, const dataArray& da)
{
    std::vector<int> j;
    std::vector<int> d;
    IDSet before, after, read, write;
    std::string par;

    app->getNewDataID(5, &d);
    app->defineData("mac_grid_1", d[0]);
    app->defineData("mpi_grid_1", d[1]);
    app->defineData("face_velocities_1", d[2]);
    app->defineData("face_velocities_ghost_1", d[3]);
    app->defineData("sim_data_1", d[4]);

    app->getNewJobID(2, &d);

    before.clear();
    after.clear();
    read.clear();
    write.clear();
    par = "";
    after.insert(j[1]);
    read.insert(d[0]);
    read.insert(d[1]);
    read.insert(d[2]);
    read.insert(d[3]);
    read.insert(d[4]);
    app->spawnJob("init", j[0], before, after, read, write, par);

    before.clear();
    after.clear();
    read.clear();
    write.clear();
    par = "";
    app->spawnJob("loop", j[1], before, after, read, write, par);
};

Init::Init(WaterApp *app, JobType type)
    : Job(app, type)
{
};

void Init::Execute(std::string params, const dataArray& da)
{
};

Loop::Loop(WaterApp *app, JobType type)
    : Job(app, type)
{
};

void Loop::Execute(std::string params, const dataArray& da)
{
};
