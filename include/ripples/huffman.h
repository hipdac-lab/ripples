/**
 *  @file Huffman.h
 *  @author Sheng Di
 *  @date Aug., 2016
 *  @brief Header file for the exponential segment constructor.
 *  (C) 2016 by Mathematics and Computer Science (MCS), Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef RIPPLES_HUFFMAN_H
#define RIPPLES_HUFFMAN_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#define NDBLP 317078 //DBLP
#define NYOUTUBE 1134890 //YOUTUBE
#define LOCALPOOL 1134890 //YOUTUBE

namespace ripples {

inline void longToBytes_bigEndian(unsigned char *b, unsigned long num) 
{
	b[0] = (unsigned char)(num>>56);
	b[1] = (unsigned char)(num>>48);
	b[2] = (unsigned char)(num>>40);
	b[3] = (unsigned char)(num>>32);
	b[4] = (unsigned char)(num>>24);
	b[5] = (unsigned char)(num>>16);
	b[6] = (unsigned char)(num>>8);
	b[7] = (unsigned char)(num);
}

typedef struct node_t {
	struct node_t *left, *right;
	size_t freq;
	char t; //in_node:0; otherwise:1
	unsigned int c;
} *node;

typedef struct HuffmanTree {
	unsigned int stateNum;
	unsigned int allNodes;
	struct node_t* pool;
	node *qqq, *qq; //the root node of the HuffmanTree is qq[1]
	int n_nodes; //n_nodes is for compression
	int qend; 
	unsigned long **code;
	unsigned char *cout;
	int n_inode; //n_inode is for decompression
	int maxBitCount;
	unsigned int maxvtx;
} HuffmanTree;

HuffmanTree* createHuffmanTree(int stateNum)
{			
	HuffmanTree *huffmanTree = (HuffmanTree*)malloc(sizeof(HuffmanTree));
	memset(huffmanTree, 0, sizeof(HuffmanTree));
	huffmanTree->stateNum = stateNum;
	huffmanTree->allNodes = 2*stateNum;
	
	huffmanTree->pool = (struct node_t*)malloc(huffmanTree->allNodes*2*sizeof(struct node_t));
	huffmanTree->qqq = (node*)malloc(huffmanTree->allNodes*2*sizeof(node));
	huffmanTree->code = (unsigned long**)malloc(huffmanTree->stateNum*sizeof(unsigned long*));
	huffmanTree->cout = (unsigned char *)malloc(huffmanTree->stateNum*sizeof(unsigned char));
	
	memset(huffmanTree->pool, 0, huffmanTree->allNodes*2*sizeof(struct node_t));
	memset(huffmanTree->qqq, 0, huffmanTree->allNodes*2*sizeof(node));
  memset(huffmanTree->code, 0, huffmanTree->stateNum*sizeof(unsigned long*));
  memset(huffmanTree->cout, 0, huffmanTree->stateNum*sizeof(unsigned char));
	huffmanTree->qq = huffmanTree->qqq - 1;
	huffmanTree->n_nodes = 0;
  huffmanTree->n_inode = 0;
  huffmanTree->qend = 1;	
	huffmanTree->maxvtx = 0;  
  return huffmanTree;
}

node new_node(HuffmanTree* huffmanTree, size_t freq, unsigned int c, node a, node b)
{
	node n = huffmanTree->pool + huffmanTree->n_nodes++;
	if (freq) 
	{
		n->c = c;
		n->freq = freq;
		n->t = 1;
	}
	else {
		n->left = a; 
		n->right = b;
		n->freq = a->freq + b->freq;
		n->t = 0;
	}
	return n;
}
 
node new_node2(HuffmanTree *huffmanTree, unsigned int c, unsigned char t)
{
	huffmanTree->pool[huffmanTree->n_nodes].c = c;
	huffmanTree->pool[huffmanTree->n_nodes].t = t;
	return huffmanTree->pool + huffmanTree->n_nodes++;
} 
 
/* priority queue */
void qinsert(HuffmanTree *huffmanTree, node n)
{
	int j, i = huffmanTree->qend++;
	while ((j = (i>>1)))  //j=i/2
	{
		if (huffmanTree->qq[j]->freq <= n->freq) break;
		huffmanTree->qq[i] = huffmanTree->qq[j], i = j;
	}
	huffmanTree->qq[i] = n;
}
 
node qremove(HuffmanTree* huffmanTree)
{
	int i, l;
	node n = huffmanTree->qq[i = 1];
	node p;
	if (huffmanTree->qend < 2) return 0;
	huffmanTree->qend --;
	huffmanTree->qq[i] = huffmanTree->qq[huffmanTree->qend];
	
	while ((l = (i<<1)) < huffmanTree->qend)  //l=(i*2)
	{
		if (l + 1 < huffmanTree->qend && huffmanTree->qq[l + 1]->freq < huffmanTree->qq[l]->freq) l++;
		if(huffmanTree->qq[i]->freq > huffmanTree->qq[l]->freq)
		{
			p = huffmanTree->qq[i];
			huffmanTree->qq[i] = huffmanTree->qq[l];
			huffmanTree->qq[l] = p;
			i = l;			
		}	
		else
		{
			break;
		}
		
	}
	
	return n;
}
 
/* walk the tree and put 0s and 1s */
/**
 * @out1 should be set to 0.
 * @out2 should be 0 as well.
 * @index: the index of the byte
 * */
void build_code(HuffmanTree *huffmanTree, node n, int len, unsigned long out1, unsigned long out2)
{
	if (n->t) {
		huffmanTree->code[n->c] = (unsigned long*)malloc(2*sizeof(unsigned long));
		if(len<=64)
		{
			(huffmanTree->code[n->c])[0] = out1 << (64 - len);
			(huffmanTree->code[n->c])[1] = out2;
		}
		else
		{
			(huffmanTree->code[n->c])[0] = out1;
			(huffmanTree->code[n->c])[1] = out2 << (128 - len);
		}
		huffmanTree->cout[n->c] = (unsigned char)len;
		return;
	}
	int index = len >> 6; //=len/64
	if(index == 0)
	{
		out1 = out1 << 1;
		out1 = out1 | 0;
		build_code(huffmanTree, n->left, len + 1, out1, 0);
		out1 = out1 | 1;
		build_code(huffmanTree, n->right, len + 1, out1, 0);		
	}
	else
	{
		if(len%64!=0)
			out2 = out2 << 1;
		out2 = out2 | 0;
		build_code(huffmanTree, n->left, len + 1, out1, out2);
		out2 = out2 | 1;
		build_code(huffmanTree, n->right, len + 1, out1, out2);	
	}
}

template <typename vertex_type, typename RRRset>
void initByRRRSets(HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets, vertex_type* maxvtx) {
  	// using vertex_type = typename GraphTy::vertex_type;
  	auto in_begin=RRRsets.begin();
  	auto in_end=RRRsets.end();
  	size_t s1 = RRRsets.size(), s2, i;
  	size_t total_rrr_size = 0;
	size_t *freq = (size_t *)malloc(huffmanTree->allNodes*sizeof(size_t));
	memset(freq, 0, huffmanTree->allNodes*sizeof(size_t));
	for (; in_begin != in_end; ++in_begin) {
		s2=std::distance(in_begin->begin(),in_begin->end());
  	total_rrr_size+=s2;
		std::for_each(in_begin->begin(), in_begin->end(),
			[&](const vertex_type v) { 
				*(freq + v) += 1; 
			});
		// std::cout<<"  "<<s2;
	}
	size_t max_freq=0;
	*maxvtx=0;
  for (i = 0; i < huffmanTree->allNodes; i++){
		if (freq[i]){
			if(freq[i]>=max_freq){
				*maxvtx=i;
				max_freq=freq[i];
			}
			qinsert(huffmanTree, new_node(huffmanTree, freq[i], i, 0, 0));
		}
	}
	
	printf("1. insert leaves, n_nodes=%d, qend=%d\n",huffmanTree->n_nodes, huffmanTree->qend);	
	while (huffmanTree->qend > 2)
		qinsert(huffmanTree, new_node(huffmanTree, 0, 0, qremove(huffmanTree), qremove(huffmanTree)));
	printf("2. insert internals, n_nodes=%d\n",huffmanTree->n_nodes);
	
	build_code(huffmanTree, huffmanTree->qq[1], 0, 0, 0);
	huffmanTree->maxvtx = *maxvtx;
	printf("3. max-freq=%d, max_vtx=%d/%d\n", max_freq, *maxvtx, huffmanTree->maxvtx);
		
	free(freq);
}

template <typename vertex_type, typename RRRset>
void initByRRRSets2(HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets, 
		vertex_type* maxvtx, std::vector<uint32_t> &globalcnt) {
  	// using vertex_type = typename GraphTy::vertex_type;
  	auto in_begin=RRRsets.begin();
  	auto in_end=RRRsets.end();
  	size_t s1 = RRRsets.size(), s2, i;
  	size_t total_rrr_size = 0;
	size_t *freq = (size_t *)malloc(huffmanTree->allNodes*sizeof(size_t));
	memset(freq, 0, huffmanTree->allNodes*sizeof(size_t));
	for (; in_begin != in_end; ++in_begin) {
		s2=std::distance(in_begin->begin(),in_begin->end());
	  	total_rrr_size+=s2;
		std::for_each(in_begin->begin(), in_begin->end(),
			[&](const vertex_type v) { 
				*(freq + v) += 1;
				globalcnt[v] += 1; 
			});
		// std::cout<<"  "<<s2;
	}
	size_t max_freq=0;
	*maxvtx=0;
  for (i = 0; i < huffmanTree->allNodes; i++){
		if (freq[i]){
			if(freq[i]>=max_freq){
				*maxvtx=i;
				max_freq=freq[i];
			}
			qinsert(huffmanTree, new_node(huffmanTree, freq[i], i, 0, 0));
		}
	}
	
	printf("1. insert leaves, n_nodes=%d, qend=%d\n",huffmanTree->n_nodes, huffmanTree->qend);	
	while (huffmanTree->qend > 2)
		qinsert(huffmanTree, new_node(huffmanTree, 0, 0, qremove(huffmanTree), qremove(huffmanTree)));
	printf("2. insert internals, n_nodes=%d\n",huffmanTree->n_nodes);
	
	build_code(huffmanTree, huffmanTree->qq[1], 0, 0, 0);
	huffmanTree->maxvtx = *maxvtx;
	printf("3. max-freq=%d, max_vtx=%d/%d\n", max_freq, *maxvtx, huffmanTree->maxvtx);
		
	free(freq);
}

template <typename vertex_type, typename RRRset>
void initByRRRSets3(HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets) {
  	// using vertex_type = typename GraphTy::vertex_type;
  	auto in_begin=RRRsets.begin();
  	auto in_end=RRRsets.end();
  	size_t s1 = RRRsets.size(), s2, i=0;
  	size_t total_rrr_size = 0;
	size_t *freq = (size_t *)malloc(huffmanTree->allNodes*sizeof(size_t));
	memset(freq, 0, huffmanTree->allNodes*sizeof(size_t));
	for (; in_begin != in_end; ++in_begin) {
		s2=std::distance(in_begin->begin(),in_begin->end());
	  	total_rrr_size+=s2;
		std::for_each(in_begin->begin(), in_begin->end(),
			[&](const vertex_type v) { 
				*(freq + v) += 1;
			});
	}
	size_t max_freq=0;
  for (i = 0; i < huffmanTree->allNodes; i++){
		if (freq[i]){
			if(freq[i]>=max_freq){
				huffmanTree->maxvtx=i;
				max_freq=freq[i];
			}
			qinsert(huffmanTree, new_node(huffmanTree, freq[i], i, 0, 0));
		}
	}
	
	printf("1. insert leaves, n_nodes=%d, qend=%d\n",huffmanTree->n_nodes, huffmanTree->qend);	
	while (huffmanTree->qend > 2)
		qinsert(huffmanTree, new_node(huffmanTree, 0, 0, qremove(huffmanTree), qremove(huffmanTree)));
	printf("2. insert internals, n_nodes=%d\n",huffmanTree->n_nodes);
	
	build_code(huffmanTree, huffmanTree->qq[1], 0, 0, 0);
	printf("3. max-freq=%d, max_vtx=%d\n", max_freq, huffmanTree->maxvtx);
		
	free(freq);
}
template <typename InItr, typename vertex_type>
void printRR(InItr in_begin, size_t length, unsigned char* out, vertex_type* cpy){
	size_t i;
	using rrr_set_type = typename std::iterator_traits<InItr>::value_type;
	for(i=0;i<length;i++){
		*(cpy+i)=*(in_begin->begin()+i);
	}
}

template <typename InItr, typename vertex_type>
void encodeRR(HuffmanTree *huffmanTree, InItr in_begin, size_t length, 
		unsigned char* out, uint32_t *encodeSize, uint32_t *code_cnt,
		vertex_type* cpy, uint32_t *copy_cnt, size_t idx){
	// using rrr_set_type = typename std::iterator_traits<InItr>::value_type;
	size_t i = 0, j=0;
	unsigned char bitSize = 0, byteSize, byteSizep;
	int state;
	unsigned char *p = out;
	int lackBits = 0;

	for (i = 0;i<length;i++) 
	{
		state = *(in_begin->begin()+i);
		bitSize = huffmanTree->cout[state];	
			
		if(bitSize<20 && bitSize){ //the state is in previous huffmanTree's code-book
			*code_cnt+=1;
			if(lackBits==0)
			{
				byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1; //it's equal to the number of bytes involved (for *outSize)
				byteSizep = bitSize/8; //it's used to move the pointer p for next data
				if(byteSize<=8)
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += byteSizep;
				}
				else //byteSize>8
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += 8;
					longToBytes_bigEndian(p, (huffmanTree->code[state])[1]);
					p += (byteSizep - 8);
				}
				*encodeSize += byteSize;
				lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
			}
			else
			{
				*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
				if(lackBits < bitSize)
				{
					p++;
					//(*outSize)++;
					long newCode = (huffmanTree->code[state])[0] << lackBits;
					longToBytes_bigEndian(p, newCode);

					if(bitSize<=64)
					{
						bitSize -= lackBits;
						byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
						byteSizep = bitSize/8;
						p += byteSizep;
						(*encodeSize)+=byteSize;
						lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
					}
					else //bitSize > 64
					{
						byteSizep = 7; //must be 7 bytes, because lackBits!=0
						p+=byteSizep;
						(*encodeSize)+=byteSize;

						bitSize -= 64;
						if(lackBits < bitSize)
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
							p++;
							//(*outSize)++;
							newCode = (huffmanTree->code[state])[1] << lackBits;
							longToBytes_bigEndian(p, newCode);
							bitSize -= lackBits;
							byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
							byteSizep = bitSize/8;
							p += byteSizep;
							(*encodeSize)+=byteSize;
							lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
						}
						else //lackBits >= bitSize
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - bitSize));
							lackBits -= bitSize;
						}
					}
				}
				else //lackBits >= bitSize
				{
					lackBits -= bitSize;
					if(lackBits==0)
						p++;
				}
			}
		}
		else{
			*copy_cnt+=1;
			cpy[j]=state;
      j++;
		}	
	}
	// if(length==100){
	// 	printf("encode=%d/%d, copy=%d/%d\n",*code_cnt,*encodeSize, *copy_cnt, *encopySize/sizeof(vertex_type));
	// }
}

template <typename InItr, typename vertex_type>
void encodeRR2(const HuffmanTree *huffmanTree, InItr in_begin, size_t length, 
		unsigned char* out, uint32_t *encodeSize, uint32_t *code_cnt,
		vertex_type* cpy, uint32_t *copy_cnt, std::vector<uint32_t> &globalcnt, vertex_type* maxvtx, bool flag_print){
	// using rrr_set_type = typename std::iterator_traits<InItr>::value_type;
	size_t i = 0, j=0;
	unsigned char bitSize = 0, byteSize, byteSizep;
	uint32_t state;
	unsigned char *p = out;
	int lackBits = 0;
	if(flag_print==1){
		std::cout<<" encode-rr2:"<<length<<":";
		for (int ii = length-200;ii<length;ii++){
			std::cout<<*(in_begin->begin()+ii)<<",";
		}
		std::cout<<std::endl;
	}
	for (i = 0;i<length;i++) 
	{
		state = *(in_begin->begin()+i);
		bitSize = huffmanTree->cout[state];	
		globalcnt[state] += 1;
		if(globalcnt[state]>=globalcnt[*maxvtx]){
			*maxvtx=state;
		}	
		if(bitSize<=32 && bitSize){ //the state is in previous huffmanTree's code-book
		// if(bitSize){ //the state is in previous huffmanTree's code-book	
			*code_cnt+=1;
			if(lackBits==0)
			{
				byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1; //it's equal to the number of bytes involved (for *outSize)
				byteSizep = bitSize/8; //it's used to move the pointer p for next data
				if(byteSize<=8)
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += byteSizep;
				}
				else //byteSize>8
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += 8;
					longToBytes_bigEndian(p, (huffmanTree->code[state])[1]);
					p += (byteSizep - 8);
				}
				*encodeSize += byteSize;
				lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
			}
			else
			{
				*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
				if(lackBits < bitSize)
				{
					p++;
					//(*outSize)++;
					long newCode = (huffmanTree->code[state])[0] << lackBits;
					longToBytes_bigEndian(p, newCode);

					if(bitSize<=64)
					{
						bitSize -= lackBits;
						byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
						byteSizep = bitSize/8;
						p += byteSizep;
						(*encodeSize)+=byteSize;
						lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
					}
					else //bitSize > 64
					{
						byteSizep = 7; //must be 7 bytes, because lackBits!=0
						p+=byteSizep;
						(*encodeSize)+=byteSize;

						bitSize -= 64;
						if(lackBits < bitSize)
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
							p++;
							//(*outSize)++;
							newCode = (huffmanTree->code[state])[1] << lackBits;
							longToBytes_bigEndian(p, newCode);
							bitSize -= lackBits;
							byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
							byteSizep = bitSize/8;
							p += byteSizep;
							(*encodeSize)+=byteSize;
							lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
						}
						else //lackBits >= bitSize
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - bitSize));
							lackBits -= bitSize;
						}
					}
				}
				else //lackBits >= bitSize
				{
					lackBits -= bitSize;
					if(lackBits==0)
						p++;
				}
			}
			if((flag_print==1)&&(i>69920)){
				std::cout<<"@"<<i<<":"<<*(in_begin->begin()+i)<<","<<state<<","<<(int)bitSize<<","<<globalcnt[state]<<","<<lackBits<<" ";
			}
		}
		else{
			*copy_cnt+=1;
			cpy[j]=state;
      		j++;
		}	
	}
	// if(length==100){
	// 	printf("encode=%d/%d, copy=%d/%d\n",*code_cnt,*encodeSize, *copy_cnt, *encopySize/sizeof(vertex_type));
	// }
}

template <typename InItr, typename vertex_type>
void encodeRR22(const HuffmanTree *huffmanTree, InItr in_begin, size_t length, 
		unsigned char* out, size_t *encodeSize, size_t *code_cnt,
		vertex_type* cpy, size_t *copy_cnt, vertex_type* maxvtx, const std::string& lossyFlag){
	size_t i = 0, j=0;
	unsigned char bitSize = 0, byteSize, byteSizep;
	uint32_t state;
	unsigned char *p = out;
	int lackBits = 0;
	auto maxp = std::find (in_begin->begin(), in_begin->begin()+length, *maxvtx);
	if (maxp != in_begin->begin()+length){
		std::iter_swap(in_begin->begin(),maxp);
	}
	*encodeSize=0;
	*code_cnt=0;
	*copy_cnt=0;
	for (i = 0;i<length;i++) 
	{
		
		state = *(in_begin->begin()+i);
		bitSize = huffmanTree->cout[state];	
		if(bitSize<=32 && bitSize){ //the state is in previous huffmanTree's code-book
		// if(bitSize){ //the state is in previous huffmanTree's code-book	
			*code_cnt+=1;
			if(lackBits==0)
			{
				byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1; //it's equal to the number of bytes involved (for *outSize)
				byteSizep = bitSize/8; //it's used to move the pointer p for next data
				if(byteSize<=8)
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += byteSizep;
				}
				else //byteSize>8
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += 8;
					longToBytes_bigEndian(p, (huffmanTree->code[state])[1]);
					p += (byteSizep - 8);
				}
				*encodeSize += byteSize;
				lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
			}
			else
			{
				*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
				if(lackBits < bitSize)
				{
					p++;
					//(*outSize)++;
					long newCode = (huffmanTree->code[state])[0] << lackBits;
					longToBytes_bigEndian(p, newCode);

					if(bitSize<=64)
					{
						bitSize -= lackBits;
						byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
						byteSizep = bitSize/8;
						p += byteSizep;
						(*encodeSize)+=byteSize;
						lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
					}
					else //bitSize > 64
					{
						byteSizep = 7; //must be 7 bytes, because lackBits!=0
						p+=byteSizep;
						(*encodeSize)+=byteSize;

						bitSize -= 64;
						if(lackBits < bitSize)
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
							p++;
							//(*outSize)++;
							newCode = (huffmanTree->code[state])[1] << lackBits;
							longToBytes_bigEndian(p, newCode);
							bitSize -= lackBits;
							byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
							byteSizep = bitSize/8;
							p += byteSizep;
							(*encodeSize)+=byteSize;
							lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
						}
						else //lackBits >= bitSize
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - bitSize));
							lackBits -= bitSize;
						}
					}
				}
				else //lackBits >= bitSize
				{
					lackBits -= bitSize;
					if(lackBits==0)
						p++;
				}
			}
		}
		else{
			*copy_cnt+=1;
			if(lossyFlag=="N"){
				cpy[j]=state;
	      		j++;
	      	}	
		}	
	}
}

template <typename vertex_type>
void encodeRR3(const HuffmanTree *huffmanTree, std::vector<vertex_type> tmpR, size_t length, 
		unsigned char* out, uint32_t *encodeSize, uint32_t *code_cnt, vertex_type* cpy, uint32_t *copy_cnt, vertex_type* maxvtx, size_t first){
	size_t i = 0, j=0;
	unsigned char bitSize = 0, byteSize, byteSizep;
	int state;
	unsigned char *p = out;
	int lackBits = 0;
	
	// if(first==6284){
	// 	std::cout<<" encode-rr3:";
	// 	for (int ii = 0;ii<length;ii++){
	// 		std::cout<<" "<<tmpR[ii];
	// 	}
	// 	std::cout<<std::endl;
	// }
	auto maxp = std::find (tmpR.begin(), tmpR.begin()+length, *maxvtx);
	if (maxp != tmpR.begin()+length){
		std::iter_swap(tmpR.begin(),maxp);
	}
	for (i = 0;i<length;i++) 
	{
		state = tmpR[i];
		bitSize = huffmanTree->cout[state];	
		if(bitSize<=32 && bitSize){ //the state is in previous huffmanTree's code-book
		// if(bitSize){ //the state is in previous huffmanTree's code-book	
			*code_cnt+=1;
			if(lackBits==0)
			{
				byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1; //it's equal to the number of bytes involved (for *outSize)
				byteSizep = bitSize/8; //it's used to move the pointer p for next data
				if(byteSize<=8)
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += byteSizep;
				}
				else //byteSize>8
				{
					longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
					p += 8;
					longToBytes_bigEndian(p, (huffmanTree->code[state])[1]);
					p += (byteSizep - 8);
				}
				*encodeSize += byteSize;
				lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
			}
			else
			{
				*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
				if(lackBits < bitSize)
				{
					p++;
					//(*outSize)++;
					long newCode = (huffmanTree->code[state])[0] << lackBits;
					longToBytes_bigEndian(p, newCode);

					if(bitSize<=64)
					{
						bitSize -= lackBits;
						byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
						byteSizep = bitSize/8;
						p += byteSizep;
						(*encodeSize)+=byteSize;
						lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
					}
					else //bitSize > 64
					{
						byteSizep = 7; //must be 7 bytes, because lackBits!=0
						p+=byteSizep;
						(*encodeSize)+=byteSize;

						bitSize -= 64;
						if(lackBits < bitSize)
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
							p++;
							//(*outSize)++;
							newCode = (huffmanTree->code[state])[1] << lackBits;
							longToBytes_bigEndian(p, newCode);
							bitSize -= lackBits;
							byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
							byteSizep = bitSize/8;
							p += byteSizep;
							(*encodeSize)+=byteSize;
							lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
						}
						else //lackBits >= bitSize
						{
							*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - bitSize));
							lackBits -= bitSize;
						}
					}
				}
				else //lackBits >= bitSize
				{
					lackBits -= bitSize;
					if(lackBits==0)
						p++;
				}
			}
		}
		else{
			*copy_cnt+=1;
			cpy[j]=state;
      		j++;
		}	
	}
	// if(length==100){
	// 	printf("encode=%d/%d, copy=%d/%d\n",*code_cnt,*encodeSize, *copy_cnt, *encopySize/sizeof(vertex_type));
	// }
}

template <typename vertex_type, typename RRRset>
void encodeRRRSets(HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets, 
	unsigned char** compR,	uint32_t* compBytes, uint32_t* codeCnt,
	vertex_type** copyR, uint32_t* copyCnt) {
  	// using vertex_type = typename GraphTy::vertex_type;
  	auto in_begin=RRRsets.begin();
  	auto in_end=RRRsets.end();
  	size_t s1 = RRRsets.size(), s2, i=0; //i: current RR's index
  	bool flag_s2=0;
  	size_t total_rrr_size = 0;
  	unsigned char* tmp_encode=NULL; 
  	vertex_type* tmp_encopy = NULL;
  	uint32_t encodeSize, code_cnt, copy_cnt;
	for (; in_begin != in_end; ++in_begin) {
		s2=std::distance(in_begin->begin(),in_begin->end());
    	total_rrr_size+=s2;
		tmp_encode = (unsigned char*)malloc(s2*sizeof(unsigned long));
    memset(tmp_encode,0,s2*sizeof(unsigned long));
		tmp_encopy = (vertex_type*)malloc(s2*sizeof(vertex_type));
    memset(tmp_encopy,0,s2*sizeof(vertex_type));
		encodeSize = 0; 
		code_cnt = 0; 
		copy_cnt = 0;
		
		encodeRR(huffmanTree, in_begin, s2, tmp_encode, &encodeSize, &code_cnt, tmp_encopy, &copy_cnt, i);
		compR[i] = (unsigned char*)malloc(encodeSize*sizeof(unsigned char));
		compBytes[i] = encodeSize;
		codeCnt[i] = code_cnt;
		memcpy(compR[i], tmp_encode, encodeSize);
		
		copyCnt[i] = copy_cnt;
    if(copy_cnt>0){
  		copyR[i] = (vertex_type*)malloc(copy_cnt*sizeof(vertex_type));
      memset(copyR[i],0,copy_cnt*sizeof(vertex_type));
  		memcpy(copyR[i], tmp_encopy, copy_cnt*sizeof(vertex_type));
			free(tmp_encopy);
		}
    if(encodeSize>0){
      free(tmp_encode);
    }
		i++;
	}
}

template <typename vertex_type, typename RRRset>
void encodeRRRSets2(const HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets, const int blockoffset,
	std::vector<unsigned char*> &compR,	std::vector<uint32_t> &compBytes, std::vector<uint32_t> &codeCnt,
	std::vector<vertex_type*> &copyR, std::vector<uint32_t> &copyCnt, std::vector<uint32_t> &globalcnt, vertex_type* maxvtx) {
  	// using vertex_type = typename GraphTy::vertex_type;
		std::cout<<"encode-rrrsets-2: huffman-maxv"<<huffmanTree->maxvtx<<"/ maxv="<<*maxvtx<<std::endl;
  	auto in_begin=RRRsets.begin() + blockoffset;
  	auto in_end=RRRsets.end();
  	size_t s1 = RRRsets.size(), s2, i=0; //i: current RR's index
  	bool flag_s2=0;
  	size_t total_rrr_size = 0;
  	unsigned char* tmp_encode=NULL; 
  	vertex_type* tmp_encopy = NULL;
  	uint32_t encodeSize, code_cnt, copy_cnt;
  	bool flag_print = 0;
	for (; in_begin != in_end; ++in_begin) {
		s2=std::distance(in_begin->begin(),in_begin->end());
    	total_rrr_size+=s2;
		tmp_encode = (unsigned char*)malloc(s2*sizeof(unsigned long));
    	memset(tmp_encode,0,s2*sizeof(unsigned long));
		tmp_encopy = (vertex_type*)malloc(s2*sizeof(vertex_type));
	    memset(tmp_encopy,0,s2*sizeof(vertex_type));
		encodeSize = 0; 
		code_cnt = 0; 
		copy_cnt = 0;
		if((blockoffset>0)&&(i==0)){
			flag_print = 1;
		}	
		encodeRR2(huffmanTree, in_begin, s2, tmp_encode, &encodeSize, &code_cnt, tmp_encopy, &copy_cnt, globalcnt, maxvtx, flag_print);
	    unsigned char* tmpComp = NULL;
	    vertex_type* tmpCopy = NULL;
	    if(encodeSize>0){
	      tmpComp = (unsigned char*)malloc(encodeSize*sizeof(unsigned char));
	      memset(tmpComp,0,encodeSize*sizeof(unsigned char));
	      memcpy(tmpComp, tmp_encode, encodeSize*sizeof(unsigned char));
	      free(tmp_encode);
	    }
	    compBytes.push_back(encodeSize);
	    codeCnt.push_back(code_cnt);
	    compR.push_back(tmpComp);

	    if(copy_cnt>0){
	  		tmpCopy = (vertex_type*)malloc(copy_cnt*sizeof(vertex_type));
	      	memset(tmpCopy,0,copy_cnt*sizeof(vertex_type));
	  		memcpy(tmpCopy, tmp_encopy, copy_cnt*sizeof(vertex_type));
			free(tmp_encopy);
		}
	    copyCnt.push_back(copy_cnt);
	    copyR.push_back(tmpCopy);
		i++;
		(*in_begin).clear();
		(*in_begin).shrink_to_fit();
		// std::cout<<"encode("<<i<<") cap="<<(*in_begin).capacity()<<std::endl;
	}
}

template <typename vertex_type, typename RRRset>
void encodeRRRSets3(const HuffmanTree* huffmanTree, std::vector<RRRset> &RRRsets, const int blockoffset,
	std::vector<unsigned char*> &compR,	std::vector<uint32_t> &compBytes, std::vector<uint32_t> &codeCnt,
	std::vector<vertex_type*> &copyR, std::vector<uint32_t> &copyCnt, std::vector<uint32_t> &globalcnt, vertex_type* maxvtx, const std::string& lossyFlag) {
  	size_t s1 = RRRsets.size(); //i: current RR's index
  	std::cout<<"encode-rrrsets-3: huffman-maxv"<<huffmanTree->maxvtx<<"/ maxv="<<*maxvtx;
  	std::cout<<" s1="<<s1<<" block-offset="<<blockoffset;
  	std::cout<<" input compR.size="<<compR.size()<<" lossy-flag="<<lossyFlag<< std::endl;
  	bool flag_s2=0;
  	size_t total_rrr_size = 0;
  	size_t num_threads = omp_get_max_threads();
  	size_t block_code_sum = 0, block_copy_sum = 0;
  	
#pragma omp parallel for num_threads(num_threads) reduction(+:total_rrr_size, block_code_sum, block_copy_sum) schedule(static)  	
    for (size_t i=blockoffset; i<s1; i++) {
        unsigned char* tmp_encode=NULL; 
        vertex_type* tmp_encopy = NULL;
        size_t encodeSize=0, code_cnt=0, copy_cnt=0;
        // auto in_begin=RRRsets.begin() + i;
        auto in_begin=RRRsets.begin();
        std::advance(in_begin, i);
        size_t s2=std::distance(in_begin->begin(),in_begin->end());
        tmp_encode = (unsigned char*)malloc(s2*sizeof(unsigned long));
        memset(tmp_encode,0,s2*sizeof(unsigned long));
        if(lossyFlag=="N"){
	        tmp_encopy = (vertex_type*)malloc(s2*sizeof(vertex_type));
	        memset(tmp_encopy,0,s2*sizeof(vertex_type));
	    }
        encodeRR22(huffmanTree, in_begin, s2, tmp_encode, &encodeSize, &code_cnt, tmp_encopy, &copy_cnt, maxvtx, lossyFlag);
        if(encodeSize>=1){
            compR[i] = (unsigned char*)malloc(encodeSize*sizeof(unsigned char));
            memset(compR[i],0,encodeSize*sizeof(unsigned char));
            memcpy(compR[i], tmp_encode, encodeSize*sizeof(unsigned char));
        }
        compBytes[i]=encodeSize;
        codeCnt[i]=code_cnt;
        if((copy_cnt>=1)&&(lossyFlag=="N")){
            copyR[i] = (vertex_type*)malloc(copy_cnt*sizeof(vertex_type));
            memset(copyR[i],0,copy_cnt*sizeof(vertex_type));
        	memcpy(copyR[i], tmp_encopy, copy_cnt*sizeof(vertex_type)); 
        }
        copyCnt[i]=copy_cnt;
		free(tmp_encode); 
		if(lossyFlag=="N"){       
	        free(tmp_encopy);
	    }
        (*in_begin).clear();	//# check why block+compress is faster than original sampling
        (*in_begin).shrink_to_fit(); //# check why block+compress is faster than original sampling
        block_code_sum += codeCnt[i];
        block_copy_sum += copyCnt[i];
        total_rrr_size += s2;
    }
	std::cout<<" compress-block code="<<block_code_sum;
	std::cout<<" copy="<<block_copy_sum<<" total=" <<total_rrr_size<<std::endl;
}

template <typename vertex_type>
void decode(unsigned char *s, size_t targetLength, node t, vertex_type *out)
{
	size_t i = 0, byteIndex = 0, count = 0;
	int r; 
	node n = t;
	if(n->t) //root->t==1 means that all state values are the same (constant)
	{
		for(count=0;count<targetLength;count++)
			out[count] = n->c;
		return;
	}
	
	for(i=0;count<targetLength;i++)
	{
		
		byteIndex = i>>3; //i/8
		r = i%8;
		if(((s[byteIndex] >> (7-r)) & 0x01) == 0)
			n = n->left;
		else
			n = n->right;

		if (n->t) {
			out[count] = n->c;
			n = t; 
			count++;
		}
	}
	// if (t != n) printf("garbage input\n");
	return;
}

template <typename vertex_type>
void decodeCheck(const unsigned char *s, const size_t targetLength, node t, vertex_type *out, const vertex_type  target, bool* find_flag)
{
	size_t i = 0, byteIndex = 0, count = 0;
	int r; 
	node n = t;
	if(n->t) //root->t==1 means that all state values are the same (constant)
	{
		for(count=0;count<targetLength;count++)
			out[count] = n->c;
		return;
	}
	
	for(i=0;count<targetLength;i++)
	{
		
		byteIndex = i>>3; //i/8
		r = i%8;
		if(((s[byteIndex] >> (7-r)) & 0x01) == 0)
			n = n->left;
		else
			n = n->right;

		if (n->t) {
			out[count] = n->c;
			if(n->c==target){
				// printf("decomp-and-check found:%d/%d/%d\n",n->c, n->freq,*freq);
				*find_flag=1;
				n = t;
				break;
			}
			n = t; 
			count++;
		}
	}
	if (t != n) printf("garbage input\n");
	return;
}

// template <typename vertex_type>
// void decode_fromTree(HuffmanTree* huffmanTree, unsigned char *s, uint32_t codeCnt, vertex_type *out)
// {
// 	node root = huffmanTree->pool+ huffmanTree->n_nodes-1;
// 	decode(s, codeCnt, root, out);
// }

// template <typename vertex_type>
// void decodeCheck_fromTree(HuffmanTree* huffmanTree, unsigned char *s, uint32_t codeCnt, vertex_type *out, vertex_type target)
// {
// 	node root = huffmanTree->pool+ huffmanTree->n_nodes-1;
// 	decodeCheck(s, codeCnt, root, out, target);
// }

template <typename vertex_type>
vertex_type DecompAndFind(HuffmanTree* huffmanTree, const uint32_t tot_nodes,
                  const unsigned char** compR, const uint32_t* compBytes, const uint32_t* codeCnt,
                  const vertex_type** copyR, const uint32_t* copyCnt, bool* deleteflag,
                  const uint32_t s1, const vertex_type maxvtx, size_t *freq,
                  IMMExecutionRecord &record, 
                  omp_parallel_tag &&ex_tag) {

  uint32_t* globalcnt=(uint32_t*)malloc(tot_nodes*sizeof(uint32_t));
  memset(globalcnt,0,tot_nodes*sizeof(uint32_t));
  uint32_t uncovered = s1;
  // size_t freq=0;
  bool find_flag;
  *freq=0;
  vertex_type* decodes = NULL;
  node hroot = huffmanTree->pool+ huffmanTree->n_nodes-1;
  // std::cout<<">decomp-find: "<<maxvtx<<"\t"; ## verify correctness of query-compressed-rr
  for(size_t i=0;i<s1;i++){
    find_flag=0;
    if(deleteflag[i]==0){
      decodes = (vertex_type*)malloc(sizeof(vertex_type)*codeCnt[i]);
      decodeCheck(compR[i], codeCnt[i], hroot, decodes, maxvtx, &find_flag);
      if(find_flag==1){
        *freq+=1;
      }
      else{
        for(int j=0;j<copyCnt[i];j++){
          if(copyR[i][j]==maxvtx){
            *freq+=1;
            find_flag=1;
            break;
          }
        }
      }
      if(find_flag==0){
        for(int j=0;j<codeCnt[i];j++){
          globalcnt[decodes[j]]+=1;
        }
        for(int j=0;j<copyCnt[i];j++){
          globalcnt[copyR[i][j]]+=1;
        }
      }
      else{
        deleteflag[i]=1;
      }  
      free(decodes);
    }
  }
  // std::cout<<" "<<*freq<<std::endl; ## verify correctness of query-compressed-rr
  uint32_t tmpmax=0;
  vertex_type nxtmax=0;
  for(int i=0;i<tot_nodes;i++){
    if(globalcnt[i]>tmpmax)  {
      tmpmax=globalcnt[i];
      nxtmax=i;
    }  
  }
  free(globalcnt);
  // std::cout<<"> nxtmx="<<nxtmax;
  // std::cout<<"> tmpfreq="<<tmpmax<<"."<<std::endl;
  
  return nxtmax;
}

template <typename vertex_type>
vertex_type DecompAndFind2(HuffmanTree* huffmanTree, const uint32_t tot_nodes,
                  const std::vector<unsigned char*> &compR, const std::vector<uint32_t> &codeCnt,
                  const std::vector<vertex_type*> &copyR, const std::vector<uint32_t> copyCnt, std::vector<bool> &deleteflag,
                  const uint32_t s1, const vertex_type maxvtx, size_t *freq,
                  IMMExecutionRecord &record, 
                  omp_parallel_tag &&ex_tag) {

  uint32_t* globalcnt=(uint32_t*)malloc(tot_nodes*sizeof(uint32_t));
  memset(globalcnt,0,tot_nodes*sizeof(uint32_t));
  uint32_t uncovered = s1;
  // size_t freq=0;
  bool find_flag;
  *freq=0;
  vertex_type* decodes = NULL;
  node hroot = huffmanTree->pool+ huffmanTree->n_nodes-1;
  // std::cout<<">decomp-find: "<<maxvtx<<"\t"; ## verify correctness of query-compressed-rr
  for(size_t i=0;i<s1;i++){
    find_flag=0;
    if(deleteflag[i]==0){
      decodes = (vertex_type*)malloc(sizeof(vertex_type)*codeCnt[i]);
      decodeCheck(compR[i], codeCnt[i], hroot, decodes, maxvtx, &find_flag);
      if(find_flag==1){
        *freq+=1;
      }
      else{
        for(int j=0;j<copyCnt[i];j++){
          if(copyR[i][j]==maxvtx){
            *freq+=1;
            find_flag=1;
            break;
          }
        }
      }
      if(find_flag==0){
        for(int j=0;j<codeCnt[i];j++){
          globalcnt[decodes[j]]+=1;
        }
        for(int j=0;j<copyCnt[i];j++){
          globalcnt[copyR[i][j]]+=1;
        }
      }
      else{
        deleteflag[i]=1;
      }  
      free(decodes);
    }
  }
  // std::cout<<" "<<*freq<<std::endl; ## verify correctness of query-compressed-rr
  uint32_t tmpmax=0;
  vertex_type nxtmax=0;
  for(int i=0;i<tot_nodes;i++){
    if(globalcnt[i]>tmpmax)  {
      tmpmax=globalcnt[i];
      nxtmax=i;
    }  
  }
  free(globalcnt);
  // std::cout<<"> nxtmx="<<nxtmax;
  // std::cout<<"> tmpfreq="<<tmpmax<<"."<<std::endl;
  
  return nxtmax;
}

template <typename vertex_type>
vertex_type DecompAndFind3(const HuffmanTree* huffmanTree, const uint32_t tot_nodes,
                  const std::vector<unsigned char*> &compR, const std::vector<uint32_t> &codeCnt,
                  const std::vector<vertex_type*> &copyR, const std::vector<uint32_t> copyCnt, 
                  std::vector<bool> &deleteflag,
                  const uint32_t s1, const vertex_type maxvtx, size_t *freq,
                  IMMExecutionRecord &record, 
                  omp_parallel_tag &&ex_tag, const std::string& lossyFlag, int release_flag) {
	*freq=0;
	size_t num_threads = omp_get_max_threads();
	// if(num_threads>=16){
	// 	num_threads=8;
	// }
	size_t local_freq=0;
	std::vector<size_t> globalcnt(tot_nodes,0);

  #pragma omp declare reduction(+ : std::vector<size_t> : \
                       std::transform(omp_out.begin(), omp_out.end(), omp_in.begin(), omp_out.begin(), std::plus<size_t>())) \
                    initializer(omp_priv = decltype(omp_orig)(omp_orig.size()))
#pragma omp parallel for num_threads(num_threads) shared(deleteflag) reduction(+:globalcnt,local_freq) //schedule(dynamic)
  for(size_t i=0;i<s1;i++){
    vertex_type* decodes = NULL;
    bool find_flag=0;
    uint32_t code_cnt=codeCnt[i];
    int threadnum = omp_get_thread_num();
    node hroot = huffmanTree->pool+ huffmanTree->n_nodes-1;
    if(deleteflag[i]==0){
    	decodes = (vertex_type*)malloc(codeCnt[i]*sizeof(vertex_type));
    	if(codeCnt[i]>0){
	    	decodeCheck(compR[i], codeCnt[i], hroot, decodes, maxvtx, &find_flag);
    	}
		if(find_flag==1){
			local_freq+=1;
		}
		else{
			if(lossyFlag=="N"){
				for(int j=0;j<copyCnt[i];j++){
					if(copyR[i][j]==maxvtx){
						local_freq+=1;
						find_flag=1;
						break;
					}
				}
			}	
		}
		if(find_flag==0){
			for(int j=0;j<codeCnt[i];j++){
				globalcnt[decodes[j]]+=1;
			}
			if(lossyFlag=="N"){
				for(int j=0;j<copyCnt[i];j++){
					globalcnt[copyR[i][j]]+=1;
				}
			}	
		}
	    else{
		    deleteflag[i]=1;
		    if(release_flag==1){
		        if(codeCnt[i]>0){
		        	free(compR[i]);
		        }
		        if(lossyFlag=="N"){
			        if(copyCnt[i]>0){
			        	free(copyR[i]);
			        }
			    }    
		    }	
	    }
	    if(code_cnt>0){
      		free(decodes);
      	}
    }
  }
  uint32_t tmpmax=0;
  vertex_type nxtmax=0;
  for(int i=0;i<tot_nodes;i++){
    if(globalcnt[i]>tmpmax)  {
      tmpmax=globalcnt[i];
      nxtmax=i;
    }  
  }
  *freq=local_freq;
  std::cout<<","<<*freq;
  globalcnt.clear();
  globalcnt.shrink_to_fit();
  return nxtmax;
}

template <typename vertex_type>
vertex_type DecompAndFind4(const HuffmanTree* huffmanTree, const uint32_t tot_nodes,
                  const std::vector<unsigned char*> &compR, const std::vector<uint32_t> &codeCnt,
                  const std::vector<vertex_type*> &copyR, const std::vector<uint32_t> copyCnt, 
                  std::vector<bool> &deleteflag,
                  const uint32_t s1, const vertex_type maxvtx, size_t *freq,
                  IMMExecutionRecord &record, 
                  omp_parallel_tag &&ex_tag, const std::string& lossyFlag, int release_flag) {
	*freq=0;
	size_t num_threads = omp_get_max_threads();
	size_t local_freq=0;
	int* globalcnt=(int*)malloc(tot_nodes*sizeof(int));
	memset(globalcnt, 0, tot_nodes*sizeof(int));
#pragma omp parallel num_threads(num_threads) proc_bind(spread) shared(deleteflag,globalcnt)//private(localcnt)//shared(deleteflag)
{
    int* localcnt=(int*)malloc(tot_nodes*sizeof(int));
    for(int ii=0;ii<tot_nodes;ii++){
        localcnt[ii]=0;
    }
#pragma omp for schedule(static) reduction(+:local_freq) 
    // parallel num_threads(num_threads) proc_bind(spread)  schedule(static)  shared(deleteflag) reduction(+:local_freq)
    for(size_t i=0;i<s1;i++){
        vertex_type* decodes = NULL;
        bool find_flag=0;
        uint32_t code_cnt=codeCnt[i];
        
        node hroot = huffmanTree->pool+ huffmanTree->n_nodes-1;

        if(deleteflag[i]==0){
        	decodes = (vertex_type*)malloc(codeCnt[i]*sizeof(vertex_type));
        	if(codeCnt[i]>0){
            	decodeCheck(compR[i], codeCnt[i], hroot, decodes, maxvtx, &find_flag);
        	}
        	if(find_flag==1){
        		local_freq+=1;
        	}
        	else{
        		if(lossyFlag=="N"){
        			for(int j=0;j<copyCnt[i];j++){
        				if(copyR[i][j]==maxvtx){
        					local_freq+=1;
        					find_flag=1;
        					break;
        				}
        			}
        		}	
        	}
        	if(find_flag==0){
        		for(int j=0;j<codeCnt[i];j++){
                    localcnt[decodes[j]]+=1;
        		}
        		if(lossyFlag=="N"){
        			for(int j=0;j<copyCnt[i];j++){
        				localcnt[copyR[i][j]]+=1;
        			}
        		}	
        	}
            else{
        	    deleteflag[i]=1;
            }
            if(code_cnt>0){
          		free(decodes);
          	}
        }
    }
    int local_vtx=0, local_max=0;
    for(int ii=0;ii<tot_nodes;ii++){
        if(localcnt[ii]>local_max)  {
          local_max=localcnt[ii];
          local_vtx=ii;
        }
    }
#pragma omp critical
    {	
    	int threadnum = omp_get_thread_num(); 
		// std::cout<<" ("<<threadnum<<") vtx="<<local_vtx<<" freq="<<local_max<<std::endl;
		globalcnt[local_vtx]+=local_max;
    }    
    free(localcnt); 
}
    uint32_t tmpmax=0;
    vertex_type nxtmax=0;
    for(int i=0;i<tot_nodes;i++){
        if(globalcnt[i]>tmpmax)  {
          tmpmax=globalcnt[i];
          nxtmax=i;
        }  
    }
    free(globalcnt);
    *freq=local_freq;
    std::cout<<","<<nxtmax<<" ";
    return nxtmax;
}

void SZ_ReleaseHuffman(HuffmanTree* huffmanTree)
{
  size_t i;
  free(huffmanTree->pool);
  huffmanTree->pool = NULL;
  free(huffmanTree->qqq);
  huffmanTree->qqq = NULL;
  for(i=0;i<huffmanTree->stateNum;i++)
  {
    if(huffmanTree->code[i]!=NULL)
      free(huffmanTree->code[i]);
  }
  free(huffmanTree->code);
  huffmanTree->code = NULL;
  free(huffmanTree->cout);
  huffmanTree->cout = NULL; 
  free(huffmanTree);
  huffmanTree = NULL;
}

template <typename GraphTy, typename ConfTy, typename RRRset>
auto HuffmanFind(HuffmanTree* huffmanTree, const GraphTy &G, const ConfTy &CFG, std::vector<RRRset> &R,
                IMMExecutionRecord &record, omp_parallel_tag &&ex_tag){
  using vertex_type = typename GraphTy::vertex_type;
  std::cout<<"*** huffman-find-1*** graph.#nodes:"<<G.num_nodes()<<std::endl;
  size_t k = CFG.k;
  std::vector<vertex_type> S2;
  
  unsigned char** compR=(unsigned char**)malloc(R.size()*sizeof(unsigned char*));
  uint32_t* compBytes=(uint32_t*)malloc(R.size()*sizeof(uint32_t));
  uint32_t* codeCnt=(uint32_t*)malloc(R.size()*sizeof(uint32_t));

  vertex_type** copyR=(vertex_type**)malloc(R.size()*sizeof(vertex_type*));
  uint32_t* copyCnt=(uint32_t*)malloc(R.size()*sizeof(uint32_t));

  vertex_type maxvtx=0, nxtmax=0;
  // double vm1,vm2;
  // process_mem_usage(vm1);
  huffmanTree = createHuffmanTree(G.num_nodes());////maxvtx-dblp:317077
  initByRRRSets<vertex_type>(huffmanTree, R, &maxvtx);
  encodeRRRSets<vertex_type>(huffmanTree, R, compR, compBytes, codeCnt, copyR, copyCnt);
  // process_mem_usage(vm2);
  // std::cout << ">>          create huffmantree. using:"<< vm1<<","<<vm2 << " Mb" <<std::endl;

  int total_copy=0;
  for(int i=0;i<R.size();i++){
    if(copyCnt[i]>0){
      total_copy+=1;
    }
  }  
  std::cout<<"(total:"<< R.size() <<"-copy:"<<total_copy<<")"<<std::endl;

  bool* deleteflag=(bool*)malloc(R.size()*sizeof(char));
  memset(deleteflag, 0, R.size()*sizeof(char));

  size_t uncovered=R.size(), freq=0;
  double f=0.0;

  while(S2.size() < CFG.k && uncovered != 0){
    S2.push_back(maxvtx);  
    nxtmax = DecompAndFind<vertex_type>(huffmanTree, G.num_nodes(),
            compR, compBytes, codeCnt,
            copyR, copyCnt, deleteflag, 
            R.size(), maxvtx, &freq,
            record, std::forward<omp_parallel_tag>(ex_tag));
    uncovered-=freq;
    maxvtx=nxtmax;
    f = double(R.size() - uncovered) / R.size();
    // std::cout<<">DecompAndFind: uncovered="<<uncovered<<" cover="<<f<<std::endl;
  }
  std::cout << ">>          release huffmantree. ("<< maxvtx<<")"<<std::endl;
  int delete_cnt=0;
  for(int i=0;i<R.size();i++){
    if(deleteflag[i])
      delete_cnt+=1;
  }
  std::cout<<" deleted-RR: "<< delete_cnt <<" f="<<f<<std::endl;
  return std::make_pair(f, S2);
}

template <typename GraphTy, typename ConfTy, typename RRRset>
auto HuffmanFind(HuffmanTree* huffmanTree, const GraphTy &G, const ConfTy &CFG, std::vector<RRRset> &R,
                IMMExecutionRecord &record, omp_parallel_tag &&ex_tag, int create_flag){
  using vertex_type = typename GraphTy::vertex_type;
  std::cout<<"*** huffman-find-2*** graph.#nodes:"<<G.num_nodes()<<std::endl;
  size_t k = CFG.k;
  std::vector<vertex_type> S2;
  
  std::vector<unsigned char*> compR;
  std::vector<uint32_t> compBytes;
  std::vector<uint32_t> codeCnt;
  std::vector<vertex_type*> copyR;
  std::vector<uint32_t> copyCnt;
  std::vector<uint32_t> globalcnt;
  globalcnt.reserve(G.num_nodes());
  for(int i=0;i<G.num_nodes();i++){
    globalcnt[i]=0;
  }

  vertex_type maxvtx=0, nxtmax=0;
  // double vm1,vm2;
  // process_mem_usage(vm1);
  if (create_flag == 1){
    huffmanTree = createHuffmanTree(G.num_nodes());////maxvtx-dblp:317077
    initByRRRSets2<vertex_type>(huffmanTree, R, &maxvtx, globalcnt);
	}
  encodeRRRSets2<vertex_type>(huffmanTree, R, 0, compR, compBytes, codeCnt, copyR, copyCnt, globalcnt, &maxvtx);
  // process_mem_usage(vm2);
  // std::cout << ">>          create huffmantree. using:"<< vm1<<","<<vm2 << " Mb" <<std::endl;

  int total_copy=0;
  for(int i=0;i<R.size();i++){
    if(copyCnt[i]>0){
      total_copy+=1;
    }
  }  
  std::cout<<"(total:"<< R.size() <<"-copy:"<<total_copy<<")"<<std::endl;

  std::vector<bool> deleteflag;
  deleteflag.reserve(R.size());
  for(int i=0;i<R.size();i++){
    deleteflag[i]=0;
  }
  size_t uncovered=R.size(), freq=0, idx=0;
  double f=0.0;
  maxvtx = huffmanTree->maxvtx;
  while(S2.size() < CFG.k && uncovered != 0){
  	if(maxvtx==0){
  		std::cout<<" **** maxvtx="<<maxvtx<<" step("<<idx<<")"<<std::endl;
  	}
    S2.push_back(maxvtx);  
    nxtmax = DecompAndFind2<vertex_type>(huffmanTree, G.num_nodes(),
            compR, codeCnt, copyR, copyCnt, deleteflag, 
            R.size(), maxvtx, &freq,
            record, std::forward<omp_parallel_tag>(ex_tag));
    uncovered-=freq;
    maxvtx=nxtmax;
    f = double(R.size() - uncovered) / R.size();
    idx++;
    // std::cout<<">DecompAndFind: uncovered="<<uncovered<<" cover="<<f<<std::endl;
  }
  std::cout << ">>          release huffmantree. ("<< maxvtx<<")"<<std::endl;
  int delete_cnt=0;
  for(int i=0;i<R.size();i++){
    if(deleteflag[i])
      delete_cnt+=1;
  }
  std::cout<<" deleted-RR: "<< delete_cnt <<" f="<<f<<std::endl;
  return std::make_pair(f, S2);
}

} //// namespace ripples
#endif //RIPPLES_HUFFMAN_H