#include "lupanels.hpp"

lpanel_t::lpanel_t(int_t *lsub, double *lval, int_t* xsup, int_t isDiagIncluded)
{
    // set the value
    val = lval;
    int_t nlb = lsub[0];
    int_t nzrow = lsub[1];
    int_t lIndexSize = LPANEL_HEADER_SIZE + 2 * nlb + 1 + nzrow;
    //
    index = (int_t*) SUPERLU_MALLOC(sizeof(int_t) * lIndexSize);
    index[0] = nlb;
    index[1] = nzrow;
    index[2] = isDiagIncluded;  //either one or zero 
    index[LPANEL_HEADER_SIZE + nlb] = 0; // starting of prefix sum is zero
    // now start the loop
    int_t blkIdPtr  = LPANEL_HEADER_SIZE;
    int_t pxSumPtr  = LPANEL_HEADER_SIZE + nlb + 1;
    int_t rowIdxPtr = LPANEL_HEADER_SIZE + 2 * nlb + 1;
    int_t lsub_ptr  = BC_HEADER;
    for (int_t lb = 0; lb < nlb; lb++)
    {
        /**
        *   BLOCK DESCRIPTOR (of size LB_DESCRIPTOR)  |
        *       block number (global)              |
        *       number of full rows in the block 
        ***/
        int_t global_id = lsub[lsub_ptr];
        int_t nrows     = lsub[lsub_ptr + 1];

        index[blkIdPtr++] = global_id;
        index[pxSumPtr] = nrows + index[pxSumPtr - 1];
        pxSumPtr++;

        int_t firstRow = xsup[global_id];
        for (int rowId = 0; rowId < nrows; rowId++)
        {
            //only storing relative distance
            index[rowIdxPtr++] = lsub[lsub_ptr + LB_DESCRIPTOR + rowId] - firstRow;
        }
        // Update the lsub_ptr
        lsub_ptr += LB_DESCRIPTOR + nrows;
    }
    return;
}

//TODO: can be optimized
int_t lpanel_t::find(int_t k)
{
    for (int_t i = 0; i < nblocks(); i++)
    {
        if (k == gid(i))
            return i;
    }
    //TODO: it shouldn't come here
    return -1;
}

int_t lpanel_t::panelSolve(int_t ksupsz, double *DiagBlk, int_t LDD)
{
    double *lPanelStPtr = blkPtr(0);
    int_t len = nzrows();
    if (haveDiag())
    {
        /* code */
        lPanelStPtr = blkPtr(1);
        len -= nbrow(0);
    }
    double alpha = 1.0;
    superlu_dtrsm("R", "U", "N", "N",
                  len, ksupsz, alpha, DiagBlk, LDD, lPanelStPtr, LDA());
}
