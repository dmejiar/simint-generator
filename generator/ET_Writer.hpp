#ifndef ETWRITER_HPP
#define ETWRITER_HPP

#include <iostream>

#include "generator/Classes.hpp"


class ET_Algorithm_Base;

class ET_Writer
{   
    public:
        ET_Writer(const ET_Algorithm_Base & et_algo); 

        void WriteET(std::ostream & os) const;

        void AddConstants(void) const;
        void DeclarePrimArrays(std::ostream & os) const;
        void DeclarePrimPointers(std::ostream & os) const;

        void WriteETFile(std::ostream & os, std::ostream & osh) const;

    private:
        ETStepList etsl_;
        QAMSet etint_;

        std::set<int> et_i_; // gets multiplied by one_over_2q

        static std::string ETStepString_(const ETStep & et);
        static std::string ETStepVar_(const Quartet & q);

        void WriteETInline_(std::ostream & os) const;
        void WriteETExternal_(std::ostream & os) const;
};

#endif
