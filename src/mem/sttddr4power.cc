/*
 * Copyright (c) 2014 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/sttddr4power.hh"

#include "base/intmath.hh"
#include "sim/core.hh"

namespace gem5
{

    STTDDR4Power::STTDDR4Power(const STTDDR4InterfaceParams &p, bool include_io) : powerlib(libDRAMPower(getMemSpec(p), include_io))
    {
    }

    Data::MemArchitectureSpec
    STTDDR4Power::getArchParams(const STTDDR4InterfaceParams &p)
    {
        Data::MemArchitectureSpec archSpec;
        archSpec.burstLength = p.burst_length;
        archSpec.nbrOfBanks = p.banks_per_rank;
        // One STTDDR4Power instance per rank, hence set this to 1
        archSpec.nbrOfRanks = 1;
        archSpec.dataRate = p.beats_per_clock;
        // For now we can ignore the number of columns and rows as they
        // are not used in the power calculation.
        archSpec.nbrOfColumns = 0;
        archSpec.nbrOfRows = 0;
        archSpec.width = p.device_bus_width;
        archSpec.nbrOfBankGroups = p.bank_groups_per_rank;
        archSpec.dll = p.dll;
        archSpec.twoVoltageDomains = hasTwoVDD(p);
        // Keep this disabled for now until the model is firmed up.
        archSpec.termination = false;
        return archSpec;
    }

    Data::MemTimingSpec
    STTDDR4Power::getTimingParams(const STTDDR4InterfaceParams &p)
    {
        // Set the values that are used for power calculations and ignore
        // the ones only used by the controller functionality in STTDDR4Power

        // All STTDDR4Power timings are in clock cycles
        Data::MemTimingSpec timingSpec;
        timingSpec.RC = divCeil((p.tRAS + p.tRP), p.tCK);
        timingSpec.RCD = divCeil(p.tRCD, p.tCK);
        timingSpec.RL = divCeil(p.tCL, p.tCK);
        timingSpec.RP = divCeil(p.tRP, p.tCK);
        timingSpec.RFC = divCeil(p.tRFC, p.tCK);
        timingSpec.ST = divCeil(p.tST, p.tCK);
        timingSpec.RAS = divCeil(p.tRAS, p.tCK);
        // Write latency is read latency - 1 cycle
        // Source: B.Jacob Memory Systems Cache, STTDDR4, Disk
        timingSpec.WL = timingSpec.RL - 1;
        timingSpec.DQSCK = 0; // ignore for now
        timingSpec.RTP = divCeil(p.tRTP, p.tCK);
        timingSpec.WR = divCeil(p.tWR, p.tCK);
        timingSpec.XP = divCeil(p.tXP, p.tCK);
        timingSpec.XPDLL = divCeil(p.tXPDLL, p.tCK);
        timingSpec.XS = divCeil(p.tXS, p.tCK);
        timingSpec.XSDLL = divCeil(p.tXSDLL, p.tCK);

        // Clock period in ns
        timingSpec.clkPeriod = (p.tCK / (double)(sim_clock::as_int::ns));
        assert(timingSpec.clkPeriod != 0);
        timingSpec.clkMhz = (1 / timingSpec.clkPeriod) * 1000;
        return timingSpec;
    }

    Data::MemPowerSpec
    STTDDR4Power::getPowerParams(const STTDDR4InterfaceParams &p)
    {
        // All STTDDR4Power currents are in mA
        Data::MemPowerSpec powerSpec;
        powerSpec.idd0 = p.IDD0 * 1000;
        powerSpec.idd02 = p.IDD02 * 1000;
        powerSpec.idd2p0 = p.IDD2P0 * 1000;
        powerSpec.idd2p02 = p.IDD2P02 * 1000;
        powerSpec.idd2p1 = p.IDD2P1 * 1000;
        powerSpec.idd2p12 = p.IDD2P12 * 1000;
        powerSpec.idd2n = p.IDD2N * 1000;
        powerSpec.idd2n2 = p.IDD2N2 * 1000;
        powerSpec.idd3p0 = p.IDD3P0 * 1000;
        powerSpec.idd3p02 = p.IDD3P02 * 1000;
        powerSpec.idd3p1 = p.IDD3P1 * 1000;
        powerSpec.idd3p12 = p.IDD3P12 * 1000;
        powerSpec.idd3n = p.IDD3N * 1000;
        powerSpec.idd3n2 = p.IDD3N2 * 1000;
        powerSpec.idd4r = p.IDD4R * 1000;
        powerSpec.idd4r2 = p.IDD4R2 * 1000;
        powerSpec.idd4w = p.IDD4W * 1000;
        powerSpec.idd4w2 = p.IDD4W2 * 1000;
        powerSpec.idd5 = p.IDD5 * 1000;
        powerSpec.idd52 = p.IDD52 * 1000;
        powerSpec.idd6 = p.IDD6 * 1000;
        powerSpec.idd62 = p.IDD62 * 1000;
        powerSpec.vdd = p.VDD;
        powerSpec.vdd2 = p.VDD2;
        return powerSpec;
    }

    Data::MemorySpecification
    STTDDR4Power::getMemSpec(const STTDDR4InterfaceParams &p)
    {
        Data::MemorySpecification memSpec;
        memSpec.memArchSpec = getArchParams(p);
        memSpec.memTimingSpec = getTimingParams(p);
        memSpec.memPowerSpec = getPowerParams(p);
        return memSpec;
    }

    bool
    STTDDR4Power::hasTwoVDD(const STTDDR4InterfaceParams &p)
    {
        return p.VDD2 == 0 ? false : true;
    }

    uint8_t
    STTDDR4Power::getDataRate(const STTDDR4InterfaceParams &p)
    {
        uint32_t burst_cycles = divCeil(p.tBURST_MAX, p.tCK);
        uint8_t data_rate = p.burst_length / burst_cycles;
        // 4 for GDDR5
        if (data_rate != 1 && data_rate != 2 && data_rate != 4 && data_rate != 8)
            fatal("Got unexpected data rate %d, should be 1 or 2 or 4 or 8\n");
        return data_rate;
    }

} // namespace gem5