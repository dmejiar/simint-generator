#include "generator/ERIGeneratorInfo.hpp"
#include "generator/ERI_HRR_Writer.hpp"
#include "generator/Printing.hpp"
#include "generator/Naming.hpp"


////////////////////////
// Base HRR Writer
////////////////////////
HRR_Writer::HRR_Writer(const HRR_Algorithm_Base & hrr_algo, const ERIGeneratorInfo & info) 
    : hrr_algo_(hrr_algo), info_(info), vinfo_(info.GetVectorInfo())
{ }


bool HRR_Writer::HasHRR(void) const
{
    return hrr_algo_.HasHRR();
}

bool HRR_Writer::HasBraHRR(void) const
{
    return hrr_algo_.HasBraHRR();
}

bool HRR_Writer::HasKetHRR(void) const
{
    return hrr_algo_.HasKetHRR();
}

ConstantMap HRR_Writer::GetConstants(void) const
{
    // by default, return empty
    return ConstantMap();
}


void HRR_Writer::WriteBraSteps_(std::ostream & os, const HRRDoubletStepList & steps,
                                const std::string & ncart_ket, const std::string & ketstr) const
{
    os << indent4 << "for(iket = 0; iket < " << ncart_ket << "; ++iket)\n";
    os << indent4 << "{\n";

    for(const auto & it : steps)
    {
        os << std::string(20, ' ') << "// " << it << "\n";
    
        const char * xyztype = "hAB_";
        const char * sign = " + ";
        if(it.steptype == RRStepType::I) // moving from J->I
            sign = " - ";

        os << std::string(20, ' ');

        os << HRRBraStepVar_(it.target, ncart_ket, ketstr);

        os << " = ";
        os << HRRBraStepVar_(it.src[0], ncart_ket, ketstr);
        os << sign << "( " << xyztype << it.xyz << " * ";
        os << HRRBraStepVar_(it.src[1], ncart_ket, ketstr);
        os << " );";
        os << "\n\n";
    }

    os << indent4 << "}\n";
    os << "\n";
}


void HRR_Writer::WriteKetSteps_(std::ostream & os, const HRRDoubletStepList & steps,
                                const std::string & ncart_bra, const std::string & brastr) const
{
        os << indent4 << "for(ibra = 0; ibra < " << ncart_bra << "; ++ibra)\n"; 
        os << indent4 << "{\n"; 
        for(const auto & it : steps)
        {
            os << std::string(20, ' ') << "// " << it << "\n";

            const char * xyztype = "hCD_";

            const char * sign = " + ";
            if(it.steptype == RRStepType::K) // Moving from L->K
                sign = " - ";

            os << std::string(20, ' ');
    
            os << HRRKetStepVar_(it.target, brastr);

            os << " = ";
            os << HRRKetStepVar_(it.src[0], brastr);
            os << sign << "( " << xyztype << it.xyz << " * ";
            os << HRRKetStepVar_(it.src[1], brastr);
            os << " );";
            os << "\n\n";
        }

        os << indent4 << "}\n"; 
}


std::string HRR_Writer::HRRBraStepVar_(const Doublet & d, const std::string & ncart_ket, 
                                       const std::string & ketstr) const
{
    std::string arrname = ArrVarName(d.left.am(), d.right.am(), ketstr);
    return StringBuilder("HRR_", arrname, "[", d.idx(), " * ", ncart_ket, " + iket]"); 
}


std::string HRR_Writer::HRRKetStepVar_(const Doublet & d, const std::string & brastr) const
{
    std::string arrname = ArrVarName(brastr, d.left.am(), d.right.am());
    return StringBuilder("HRR_", arrname, "[ibra * ", d.ncart(), " + ", d.idx(), "]"); 
}


////////////////////////
// Inline HRR Writer
////////////////////////

void HRR_Writer_Inline::WriteHRR(std::ostream & os) const
{
    QAM finalam = info_.FinalAM();
    DAM finalbra{finalam[0], finalam[1]};

    os << indent3 << "//////////////////////////////////////////////\n";
    os << indent3 << "// Contracted integrals: Horizontal recurrance\n";
    os << indent3 << "//////////////////////////////////////////////\n";
    os << "\n";

    if(HasBraHRR())
    {
        os << "\n";
        os << indent4 << "const double hAB_x = P.AB_x[ab];\n";
        os << indent4 << "const double hAB_y = P.AB_y[ab];\n";
        os << indent4 << "const double hAB_z = P.AB_z[ab];\n";
        os << "\n";
        os << "\n";
    }


    os << indent3 << "for(abcd = 0; abcd < nshellbatch; ++abcd, ++real_abcd)\n";
    os << indent3 << "{\n";

    os << "\n";
    os << indent4 << "// set up HRR pointers\n";
    for(const auto & it : hrr_algo_.TopAM())
        os << indent4 << "double const * restrict " << HRRVarName(it) << " = " << ArrVarName(it) << " + abcd * " << NCART(it) << ";\n";

    // and also for the final integral
    os << indent4 << "double * restrict " << HRRVarName(finalam) << " = " << ArrVarName(finalam) << " + real_abcd * " << NCART(finalam) << ";\n";
    os << "\n";



    if(HasBraHRR())
    {
        for(const auto & itk : hrr_algo_.TopKetAM()) // for all needed ket am
        for(const auto & itb : hrr_algo_.GetBraAMOrder()) // form these
        {
            QAM am = {itb[0], itb[1], itk[0], itk[1]};
            os << indent4 << "// form " << ArrVarName(am) << "\n";

            // allocate temporary if needed
            if(!info_.IsContQ(am) && !info_.IsFinalAM(am))
                os << indent4 << "double " << HRRVarName(am) << "[" << NCART(am) << "];\n";

            // ncart_ket in string form
            std::string ncart_ket_str = StringBuilder(NCART(itk[0], itk[1]));

            // the ket part in string form
            std::string ket_str = StringBuilder(amchar[itk[0]], "_", amchar[itk[1]]);
    
            // actually write out the steps now
            WriteBraSteps_(os, hrr_algo_.GetBraSteps(itb), ncart_ket_str, ket_str);
        }
    }

    os << "\n";
    os << "\n";

    if(HasKetHRR())
    {
        os << "\n";
        os << indent4 << "const double hCD_x = Q.AB_x[cd+abcd];\n";
        os << indent4 << "const double hCD_y = Q.AB_y[cd+abcd];\n";
        os << indent4 << "const double hCD_z = Q.AB_z[cd+abcd];\n";
        os << "\n";

        for(const auto & itk : hrr_algo_.GetKetAMOrder())
        {
            QAM am = {finalbra[0], finalbra[1], itk[0], itk[1]};
            os << indent4 << "// form " << ArrVarName(am) << "\n";

            // allocate temporary if needed
            if(!info_.IsContQ(am) && !info_.IsFinalAM(am))
                os << indent4 << "double " << HRRVarName(am) << "[" << NCART(am) << "];\n";

            // ncart_bra in string form
            std::string ncart_bra_str = StringBuilder(NCART(finalbra));

            // the bra part in string form
            std::string bra_str = StringBuilder(amchar[finalbra[0]], "_", amchar[finalbra[1]]);

            WriteKetSteps_(os, hrr_algo_.GetKetSteps(itk), ncart_bra_str, bra_str);
        }
    }

    os << "\n";
    os << indent3 << "}  // close HRR loop\n";

    os << "\n";
    os << "\n";
}


void HRR_Writer_Inline::WriteHRRFile(std::ostream & ofb, std::ostream & ofk, std::ostream & ofh) const
{
    
}

////////////////////////
// External HRR Writer
////////////////////////

void HRR_Writer_External::WriteHRR(std::ostream & os) const
{
    QAM finalam = info_.FinalAM();
    DAM finalbra{finalam[0], finalam[1]};


    RRStepType brasteptype = (finalam[1] > finalam[0] ? RRStepType::J : RRStepType::I);
    const char * steptypestr = (brasteptype == RRStepType::J ? "J" : "I");

    os << indent3 << "//////////////////////////////////////////////\n";
    os << indent3 << "// Contracted integrals: Horizontal recurrance\n";
    os << indent3 << "//////////////////////////////////////////////\n";
    os << "\n";

    if(HasBraHRR())
    {
        os << "\n";
        os << indent4 << "const double hAB_x = P.AB_x[ab];\n";
        os << indent4 << "const double hAB_y = P.AB_y[ab];\n";
        os << indent4 << "const double hAB_z = P.AB_z[ab];\n";
        os << "\n";
    }

    os << indent3 << "for(abcd = 0; abcd < nshellbatch; ++abcd, ++real_abcd)\n";
    os << indent3 << "{\n";

    os << "\n";
    os << indent4 << "// set up HRR pointers\n";
    for(const auto & it : hrr_algo_.TopAM())
        os << indent4 << "double const * restrict " << HRRVarName(it) << " = " << ArrVarName(it) << " + abcd * " << NCART(it) << ";\n";

    // and also for the final integral
    os << indent4 << "double * restrict " << HRRVarName(finalam) << " = " << ArrVarName(finalam) << " + real_abcd * " << NCART(finalam) << ";\n";
    os << "\n";



    if(HasBraHRR())
    {
        for(const auto & itk : hrr_algo_.TopKetAM()) // for all needed ket am
        for(const auto & itb : hrr_algo_.GetBraAMOrder()) // form these
        {
            QAM am = {itb[0], itb[1], itk[0], itk[1]};
            os << indent4 << "// form " << ArrVarName(am) << "\n";

            // allocate temporary if needed
            if(!info_.IsContQ(am) && !info_.IsFinalAM(am))
                os << indent4 << "double " << HRRVarName(am) << "[" << NCART(am) << "];\n";

            // call function
            os << indent4 << "HRR_BRA_" << steptypestr << "_" << amchar[itb[0]] << "_" << amchar[itb[1]] << "(\n";

            // pointer to result buffer
            os << indent5 << "" << HRRVarName(am) << ",\n";

            // pointer to requirements
            for(const auto & it : hrr_algo_.GetBraAMReq(itb))
                os << indent5 << "" << HRRVarName({it[0], it[1], itk[0], itk[1]}) << ",\n";

            os << indent5 << "hAB_x, hAB_y, hAB_z, " << NCART(itk[0], itk[1]) << ");\n"; 
            os << "\n\n";
        }
    }

    os << "\n";
    os << "\n";

    if(HasKetHRR())
    {
        RRStepType ketsteptype = (finalam[3] > finalam[2] ? RRStepType::L : RRStepType::K);
        const char * steptypestr = (ketsteptype == RRStepType::L ? "L" : "K");

        os << "\n";
        os << indent4 << "const double hCD_x = Q.AB_x[cd+abcd];\n";
        os << indent4 << "const double hCD_y = Q.AB_y[cd+abcd];\n";
        os << indent4 << "const double hCD_z = Q.AB_z[cd+abcd];\n";
        os << "\n";

        for(const auto & itk : hrr_algo_.GetKetAMOrder())
        {
            QAM am = {finalbra[0], finalbra[1], itk[0], itk[1]};
            os << indent4 << "// form " << ArrVarName(am) << "\n";

            // allocate temporary if needed
            if(!info_.IsContQ(am) && !info_.IsFinalAM(am))
                os << indent4 << "double " << HRRVarName(am) << "[" << NCART(am) << "];\n";

            os << indent4 << "HRR_KET_" << steptypestr << "_";
            os << amchar[itk[0]] << "_" << amchar[itk[1]] << "(\n";

            // pointer to result buffer
            os << indent5 << "" << HRRVarName(am) << ",\n";

            // pointer to requirements
            for(const auto & it : hrr_algo_.GetKetAMReq(itk))
                os << indent5 << "" << HRRVarName({finalbra[0], finalbra[1], it[0], it[1]}) << ",\n";

            os << indent5 << "hCD_x, hCD_y, hCD_z, " << NCART(finalbra) << ");\n";
            os << "\n\n";

        }
    }

    os << "\n";
    os << indent3 << "}  // close HRR loop\n";


    os << "\n";
    os << "\n";
}


void HRR_Writer_External::WriteHRRFile(std::ostream & ofb,
                                       std::ostream & ofk, std::ostream & ofh) const
{
    QAM finalam = info_.FinalAM();
    DAM braam = {finalam[0], finalam[1]};
    DAM ketam = {finalam[2], finalam[3]};

    // only call the function for the last doublets
    HRRDoubletStepList brasteps = hrr_algo_.GetBraSteps(braam);
    HRRDoubletStepList ketsteps = hrr_algo_.GetKetSteps(ketam);

    if(brasteps.size() > 0)
    {
        RRStepType brasteptype = (finalam[1] > finalam[0] ? RRStepType::J : RRStepType::I);
        const char * steptypestr = (brasteptype == RRStepType::J ? "J" : "I");

        ofb << "//////////////////////////////////////////////\n";
        ofb << "// BRA: ( " << amchar[braam[0]] << " " << amchar[braam[1]] << " |\n";
        ofb << "// Steps: " << brasteps.size() << "\n";
        ofb << "//////////////////////////////////////////////\n";
        ofb << "\n";

        std::stringstream prototype;

        prototype << "void HRR_BRA_" << steptypestr << "_";
        prototype << amchar[braam[0]] << "_" << amchar[braam[1]] << "(\n";

        // pointer to result buffer
        prototype << indent5 << "double * const restrict " << HRRVarName(finalam[0], finalam[1], "X_X") << ",\n";

        // pointer to requirements
        for(const auto & it : hrr_algo_.GetBraAMReq(braam))
            prototype << indent5 << "double const * const restrict " << HRRVarName(it[0], it[1], "X_X") << ",\n";

        prototype << indent5 << "const double hAB_x, const double hAB_y, const double hAB_z, const int ncart_ket)";



        ofb << prototype.str() << "\n";
        ofb << "{\n";
        ofb << indent1 << "int iket;\n";
        ofb << "\n";

        WriteBraSteps_(ofb, brasteps, "ncart_ket", "X_X"); 

        ofb << "\n";
        ofb << "}\n";
        ofb << "\n";
        ofb << "\n";


        // header
        ofh << prototype.str() << ";\n\n";

    }

    if(ketsteps.size() > 0)
    {
        RRStepType ketsteptype = (finalam[3] > finalam[2] ? RRStepType::L : RRStepType::K);
        const char * steptypestr = (ketsteptype == RRStepType::L ? "L" : "K");

        ofk << "//////////////////////////////////////////////\n";
        ofk << "// KET: ( " << amchar[ketam[0]] << " " << amchar[ketam[1]] << " |\n";
        ofk << "// Steps: " << ketsteps.size() << "\n";
        ofk << "//////////////////////////////////////////////\n";
        ofk << "\n";

        std::stringstream prototype;

        prototype << "void HRR_KET_" << steptypestr << "_";
        prototype << amchar[ketam[0]] << "_" << amchar[ketam[1]] << "(\n";

        // pointer to result buffer
        prototype << indent5 << "double * const restrict " << HRRVarName("X_X", finalam[2], finalam[3]) << ",\n";

        // pointer to requirements
        for(const auto & it : hrr_algo_.GetKetAMReq(ketam))
            prototype << indent5 << "double const * const restrict " << HRRVarName("X_X", it[0], it[1]) << ",\n";

        prototype << indent5 << "const double hCD_x, const double hCD_y, const double hCD_z, const int ncart_bra)\n";



        ofk << prototype.str() << "\n";
        ofk << "{\n";
        ofk << indent1 << "int ibra;\n";
        ofk << "\n";

        WriteKetSteps_(ofk, ketsteps, "ncart_bra", "X_X"); 

        ofk << "\n";
        ofk << "}\n";
        ofk << "\n";
        ofk << "\n";


        // header
        ofh << prototype.str() << ";\n";
    }
}

