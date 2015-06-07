#ifndef MAKOWSKI_HPP
#define MAKOWSKI_HPP

#include <stdexcept>
#include <algorithm>

#include "generator/VRR_Algorithm_Base.hpp" 
#include "generator/ET_Algorithm_Base.hpp" 
#include "generator/HRR_Algorithm_Base.hpp" 

class Makowski_HRR : public HRR_Algorithm_Base
{
    private:
        virtual HRRDoubletStep doubletstep(const Doublet & target)
        {
            if(target.am() == 0)
                throw std::runtime_error("Cannot HRR step to an s doublet!");

            // idx is the xyz index
            ExpList ijk = target.right.ijk;
            std::sort(ijk.begin(), ijk.end());
            auto v = std::find_if(ijk.begin(), ijk.end(), [](int i) { return i != 0; });
            auto it = std::find(target.right.ijk.rbegin(), target.right.ijk.rend(), *v); 
            int idx = 2 - std::distance(target.right.ijk.rbegin(), it);  // remember we are working with reverse iterators

            // gaussian common to both src1 and src2
            Gaussian common(target.right.StepDown(idx, 1));

            // for src1
            Gaussian src1g(target.left.StepUp(idx, 1));

            // create new doublets
            Doublet src1d{target.type, src1g, common, 0};
            Doublet src2d{target.type, target.left, common, 0};
           
            XYZStep xyzstep = IdxToXYZStep(idx);

            // Create the HRR doublet step
            HRRDoubletStep hrr{target, 
                               src1d, src2d,
                               xyzstep};
            return hrr;
        }
};

class Makowski_VRR : public VRR_Algorithm_Base
{
    public:
        virtual VRRMap CreateVRRMap(int am)
        {
            VRRMap vm;

            if(am == 0)
                return vm;  // empty!

            Gaussian g{am, 0, 0};

            do {      
                // lowest exponent, favoring the far right if equal
                // idx is the xyz index
                ExpList ijk = g.ijk;
                std::sort(ijk.begin(), ijk.end());
                auto v = std::find_if(ijk.begin(), ijk.end(), [](int i) { return i != 0; });
                auto it = std::find(g.ijk.rbegin(), g.ijk.rend(), *v); 
                int idx = 2 - std::distance(g.ijk.rbegin(), it);  // remember we are working with reverse iterators

                vm[g] = IdxToXYZStep(idx);
            } while(g.Iterate());

            return vm;
            
        }
};


class Makowski_ET : public ET_Algorithm_Base
{
    private:

        virtual ETStep etstep(const Quartet & target)
        {
            if(target.am() == 0)
                throw std::runtime_error("Cannot ET step to an s doublet!");

            // idx is the xyz index
            ExpList ijk = target.ket.left.ijk;
            std::sort(ijk.begin(), ijk.end());
            auto v = std::find_if(ijk.begin(), ijk.end(), [](int i) { return i != 0; });
            auto it = std::find(target.ket.left.ijk.rbegin(), target.ket.left.ijk.rend(), *v); 
            int idx = 2 - std::distance(target.ket.left.ijk.rbegin(), it);  // remember we are working with reverse iterators

            Gaussian src1g_1(target.bra.left);
            Gaussian src1g_2(target.ket.left.StepDown(idx, 1));

            Gaussian src2g_1(target.bra.left.StepDown(idx, 1));
            Gaussian src2g_2(target.ket.left.StepDown(idx, 1));

            Gaussian src3g_1(target.bra.left);
            Gaussian src3g_2(target.ket.left.StepDown(idx, 2));

            Gaussian src4g_1(target.bra.left.StepUp(idx, 1));
            Gaussian src4g_2(target.ket.left.StepDown(idx, 1));
            

            // create new doublets
            Gaussian s{0,0,0};

            Doublet src1d_bra{DoubletType::BRA, src1g_1, s};
            Doublet src1d_ket{DoubletType::KET, src1g_2, s};

            Doublet src2d_bra{DoubletType::BRA, src2g_1, s};
            Doublet src2d_ket{DoubletType::KET, src2g_2, s};

            Doublet src3d_bra{DoubletType::BRA, src3g_1, s};
            Doublet src3d_ket{DoubletType::KET, src3g_2, s};

            Doublet src4d_bra{DoubletType::BRA, src4g_1, s};
            Doublet src4d_ket{DoubletType::KET, src4g_2, s};



            XYZStep xyzstep = IdxToXYZStep(idx);

            // Create the electron transfer step
            ETStep et{target, 
                      {src1d_bra, src1d_ket, 0, 0},
                      {src2d_bra, src2d_ket, 0, 0},
                      {src3d_bra, src3d_ket, 0, 0},
                      {src4d_bra, src4d_ket, 0, 0},
                      xyzstep};
            return et;
        }
};

#endif
