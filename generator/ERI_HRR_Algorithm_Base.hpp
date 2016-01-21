#ifndef SIMINT_GUARD_GENERATOR__HRR_ALGORITHM_BASE_HPP_
#define SIMINT_GUARD_GENERATOR__HRR_ALGORITHM_BASE_HPP_

#include "generator/Classes.hpp"
#include "generator/Options.hpp"


typedef std::vector<HRRDoubletStep> HRRDoubletStepList;
typedef std::map<DAM, HRRDoubletStepList> HRR_StepMap;
typedef std::map<DAM, DAMSet> HRR_AMReqMap;

class HRR_Algorithm_Base
{
    public:
        HRR_Algorithm_Base(const OptionMap & options);

        void Create(QAM am);
        
        DAMSet TopBraAM(void) const;
        DAMSet TopKetAM(void) const;
        QAMSet TopAM(void) const;
        QuartetSet TopQuartets(void) const;

        DAMList GetBraAMOrder(void) const;
        DAMList GetKetAMOrder(void) const;

        DAMSet GetBraAMReq(DAM am) const;
        DAMSet GetKetAMReq(DAM am) const;
        DoubletSet TopBraDoublets(void) const;
        DoubletSet TopKetDoublets(void) const;

        HRRDoubletStepList GetBraSteps(DAM am) const;
        HRRDoubletStepList GetKetSteps(DAM am) const;

        bool HasHRR(void) const;
        bool HasBraHRR(void) const;
        bool HasKetHRR(void) const;


        virtual ~HRR_Algorithm_Base() = default;

    protected:
       int GetOption(Option opt) const; 

    private:
        // Options
        OptionMap options_;

        QAM finalam_;

        DAMSet allbraam_, allketam_;

        HRR_StepMap brasteps_, ketsteps_;
        HRR_AMReqMap brareq_, ketreq_;

        // with full quartets (not am)
        DoubletSet bratop_, kettop_;
        QuartetSet topquartets_;

        DAMSet bratopam_, kettopam_;
        QAMSet topqam_;

        DAMList braamorder_, ketamorder_;

        virtual HRRDoubletStep DoubletStep_(const Doublet & target, RRStepType steptype) = 0;

        void HRRDoubletLoop_(HRRDoubletStepList & hrrlist,
                             const DoubletSet & inittargets,
                             DoubletSet & solveddoublets,
                             DoubletSet & pruned,
                             RRStepType steptype);

        static void PruneDoublets_(DoubletSet & d, DoubletSet & pruned, RRStepType steptype);

        void AMOrder_AddWithDependencies_(DAMList & order, DAM am, DoubletType type) const;
 
};


#endif