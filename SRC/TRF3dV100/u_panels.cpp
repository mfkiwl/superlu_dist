#include "lupanels.hpp"

upanel_t::upanel_t(int_t k, int_t *usub, double *uval, int_t *xsup)
{
    int_t kSupSz = SuperSize(k);
    int_t kLastRow = xsup[k + 1];
    /* compute number of columns */
    int_t nonZeroCols = 0;
    int_t usubPtr = BR_HEADER;
    int_t nub = usub[0];

    for (int_t ub = 0; ub < nub; ub++)
    {
        int_t gblockId = usub[usubPtr];
        int_t gsupc = SuperSize(gblockId);
        for (int_t col = 0; col < gsupc; col++)
        {
            int_t segsize = kLastRow - usub[usubPtr + UB_DESCRIPTOR + col];
            if (segsize)
                nonZeroCols++;
        }

        usubPtr += UB_DESCRIPTOR + gsupc;
    }

    int_t uIndexSize = UPANEL_HEADER_SIZE + 2 * nub + 1 + nonZeroCols;
    //Allocating the index and val
    index = (int_t*) SUPERLU_MALLOC(sizeof(int_t) * uIndexSize);
    val = (double *)SUPERLU_MALLOC(sizeof(double) * nonZeroCols * kSupSz);
    index[0] = nub;
    index[1] = nonZeroCols;
    index[2] = kSupSz;
    index[UPANEL_HEADER_SIZE + nub] = 0; // starting of prefix sum is zero
    // now start the loop
    int_t blkIdPtr   = UPANEL_HEADER_SIZE;
    int_t pxSumPtr   = UPANEL_HEADER_SIZE + nub + 1;
    int_t colIdxPtr  = UPANEL_HEADER_SIZE + 2 * nub + 1;
    int_t srcUvalPtr = 0;
    int_t dstUvalPtr = 0;
    // reset the USUB ptr
    usubPtr = BR_HEADER;
    for (int_t ub = 0; ub < nub; ub++)
    {
        int_t gblockId = usub[usubPtr];
        index[blkIdPtr++] = gblockId;
        int_t local_nzcols = 0;
        int_t gsupc = SuperSize(gblockId);
        for (int_t col = 0; col < gsupc; col++)
        {
            int_t segsize = kLastRow - usub[usubPtr + UB_DESCRIPTOR + col];
            if (segsize)
            {
                for(int row=0; row<kSupSz; row++)
                {
                    if(row<kSupSz-segsize)
                        val[dstUvalPtr++] =0.0;
                    else 
                        val[dstUvalPtr++] =uval[srcUvalPtr++];
                }
                
                index[colIdxPtr++] = col; 
                local_nzcols++;
            }
        }
        index[pxSumPtr] = index[pxSumPtr - 1] + local_nzcols;
        pxSumPtr++;
        usubPtr += UB_DESCRIPTOR + gsupc;
    }

    return;
}

int_t upanel_t::find(int_t k)
{
    //TODO: possible to optimize
    for (int_t i = 0; i < nblocks(); i++)
    {
        if (k == gid(i))
            return i;
    }
    //TODO: it shouldn't come here
    return -1;
}
int_t upanel_t::panelSolve(int_t ksupsz, double *DiagBlk, int_t LDD)
{
    superlu_dtrsm("L", "L", "N", "U",
                  ksupsz, nzcols(), 1.0, DiagBlk, LDD, val, LDA());
    return 0;
}

int_t upanel_t::packed2skyline(int_t *usub, double *uval)
{
    return 0;
}
