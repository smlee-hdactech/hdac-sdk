#ifndef HDACSCRIPT_H
#define HDACSCRIPT_H

const unsigned char *mc_ParseOpDropOpReturnScript(const unsigned char *src,int size,int *op_drop_offset,int *op_drop_size,int op_drop_count,int *op_return_offset,int *op_return_size);

#endif // HDACSCRIPT_H
