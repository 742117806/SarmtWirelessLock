#include "rsa.h"

//uint8_t temp_val[RsaByte_SizeMax];

uint8_t LargeNumber_Comp(uint8_t *p_first, uint8_t *p_second, uint8_t bit_size)
{
    uint8_t i;

    for (i = 0; i < bit_size; i++)
    {
        if (p_first[i] > p_second[i])
            return 1;
        else if (p_first[i] < p_second[i])
            return 2;
    }
    return 0;
}

uint8_t LargeNumber_Size(uint8_t *p_integer, uint8_t bit_size)
{
    uint8_t i;

    for (i = 0; i < bit_size; i++)
    {
        if (p_integer[i] > 0)
            return (bit_size - i);
    }
    return 0;
}

void LargeNumber_MoveHigh_1bit(uint8_t p_sourse[RsaByte_SizeMax])
{
    uint8_t i;

    for (i = 0; i < (RsaByte_SizeMax - 1); i++)
    {
        p_sourse[i] = p_sourse[i + 1];
    }
    p_sourse[RsaByte_SizeMax - 1] = 0;
}

void LargeNumber_SameMod(uint8_t *p_sourse, uint8_t *p_subtrahend, uint8_t bit_size)
{
    while (LargeNumber_Comp(p_sourse, p_subtrahend, bit_size) != 2)
    {
        uint8_t i, jiewei = 0;
        uint16_t temp_val = 0;

        for (i = bit_size; i > 0; i--)
        {
            uint8_t sub_val;

            temp_val = p_sourse[i - 1];
            if (temp_val == p_subtrahend[i - 1])
            {
                if (jiewei == 1)
                {
                    sub_val = (temp_val + 0xff) - p_subtrahend[i - 1];
                    //jiewei = 1;
                }
                else
                {
                    sub_val = 0;
                    //jiewei = 0;
                }
            }
            else if (temp_val > p_subtrahend[i - 1])
            {

                sub_val = (temp_val - jiewei) - p_subtrahend[i - 1];
                jiewei = 0;
            }
            else
            {
                sub_val = (temp_val + 0x0100 - jiewei) - p_subtrahend[i - 1];
                jiewei = 1;
            }
            p_sourse[i - 1] = sub_val;
        }
    }
}

void LargeNumber_Modulo(uint8_t dividend[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax])
{
    uint8_t i, move_weishu;

    move_weishu = RsaByte_SizeMax - LargeNumber_Size(divisor, RsaByte_SizeMax);
    //LargeNumber_MoveHigh(divisor, move_weishu);
    LargeNumber_SameMod(dividend, &divisor[move_weishu], RsaByte_SizeMax - move_weishu);
    for (i = 0; i < move_weishu; i++)
    {
        //LargeNumber_MoveLow_1bit(&divisor[i], RsaByte_SizeMax - move_weishu);
        LargeNumber_SameMod(&dividend[i], &divisor[move_weishu - 1], RsaByte_SizeMax - move_weishu + 1);
    }
}

void LargeNumber_Mul(uint8_t mul_factor1[RsaByte_Size], uint8_t mul_factor2, uint8_t mul_Product[RsaByte_SizeMax], uint8_t flag)
{

    uint8_t i, jinwei = 0;
    uint16_t chen_ji;
    //uint8_t *p = (uint8_t *)(&chen_ji);

    if (flag == 1)
    {
        for (i = RsaByte_Size; i > 0; i--)
        {
            chen_ji = mul_factor1[i - 1] * mul_factor2 + jinwei + mul_Product[i];
            //mul_Product[i] = p[0];
            //jinwei = p[1];
            mul_Product[i] = chen_ji & 0xff;
            jinwei = chen_ji >> 8;
        }
        mul_Product[0] = jinwei + mul_Product[0];
    }
    else
    {
        for (i = RsaByte_Size; i > 0; i--)
        {
            chen_ji = mul_factor1[i - 1] * mul_factor2 + jinwei;
            //mul_Product[i] = p[0];
            //jinwei = p[1];
            mul_Product[i] = chen_ji & 0xff;
            jinwei = chen_ji >> 8;
        }
        mul_Product[0] = jinwei;
    }
}

void LargeNumber_Mul_Mod(uint8_t mul_factor1[RsaByte_Size], uint8_t mul_factor2[RsaByte_Size], uint8_t mul_Product[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax])
{
    uint8_t i, bit_size;
    uint8_t *p;

    bit_size = LargeNumber_Size(mul_factor2, RsaByte_Size);
    p = &mul_factor2[RsaByte_Size - bit_size];

    for (i = 0; i < RsaByte_SizeMax; i++)
    {
        mul_Product[i] = 0;
    }

    for (i = 0; i < bit_size - 1; i++)
    {
        LargeNumber_Mul(mul_factor1, p[i], mul_Product, 1);
        LargeNumber_Modulo(mul_Product, divisor);
        //LargeNumber_Mul(mul_Product, 0xFF, mul_Product, 0);
        LargeNumber_MoveHigh_1bit(mul_Product);
        LargeNumber_Modulo(mul_Product, divisor);
    }
    LargeNumber_Mul(mul_factor1, p[i], mul_Product, 1);
    LargeNumber_Modulo(mul_Product, divisor);
}

uint8_t index_cnt;

void LargeNumber_Power_Mod(uint8_t model[RsaByte_Size], uint8_t radix[RsaByte_Size], uint8_t index[RsaByte_Size], uint8_t mul_Product[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax])
{
    uint8_t i, j, temp_val, bit_size, byte0_cnt = 8;
    uint8_t *p;

    bit_size = LargeNumber_Size(index, RsaByte_Size);
    if (bit_size == 0)
        return;

#if (0)
    for (i = 0; i < RsaByte_Size - 1; i++)
    {
        model[i] = 0;
    }
    model[RsaByte_Size - 1] = 1;
#else
    memmove(model, radix, RsaByte_Size);
#endif
    ////////////////////////////////////////

    p = &index[RsaByte_Size - bit_size];

    temp_val = p[0];
    for (i = 0; i < 8; i++)
    {
        if (temp_val & 0x80)
            break;
        else
        {
            temp_val <<= 1;
            byte0_cnt--;
        }
    }
#if (1)
    temp_val <<= 1;
    byte0_cnt--;
#endif

    for (i = 0; i < byte0_cnt; i++)
    {
        //IWDG_FEED();
        LargeNumber_Mul_Mod(model, model, mul_Product, divisor);
        memmove(model, &mul_Product[1], RsaByte_Size);
        //while(1);
        if (temp_val & 0x80)
        {
            LargeNumber_Mul_Mod(radix, model, mul_Product, divisor);
            memmove(model, &mul_Product[1], RsaByte_Size);
        }
        temp_val <<= 1;
    } //////////////

    p++;
    for (index_cnt = 0; index_cnt < (bit_size - 1); index_cnt++)
    {
        temp_val = p[index_cnt];
        for (j = 0; j < 8; j++)
        {
            //      IWDG_FEED();
            LargeNumber_Mul_Mod(model, model, mul_Product, divisor);
            memmove(model, &mul_Product[1], RsaByte_Size);
            if (temp_val & 0x80)
            {
                LargeNumber_Mul_Mod(radix, model, mul_Product, divisor);
                memmove(model, &mul_Product[1], RsaByte_Size);
            }
            temp_val <<= 1;
        }
    }
}

/*

file://�������
file://������ʽ��N.Add(A)������ֵ��N+A
file://��������������ͬ����ֵ��ӣ�����ı���������ٵ��ô����������
/ ****************************************************************** /
���磺
     A  B  C
+       D  E
--------------
= S  F  G  H

���У���C+E<=0xffffffff����H=C+E��carry(��λ��־)=0
     ��C+E>0xffffffff����H=C+E-0x100000000��carry=1

     ��B+D+carry<=0xfffffff����G=B+D��carry=0      
     ��B+D+carry>0xfffffff����G=B+D+carry-0x10000000��carry=1

     ��carry=0����F=A��S=0
     ��carry=1��A<0xfffffff����F=A+1��S=0
     ��carry=1��A=0xfffffff����F=0��S=1
/ ***************************************************************** /
CBigInt CBigInt::Add(CBigInt& A)
{
CBigInt X;
if(X.m_nSign==A.m_nSign) 
{
X.Mov(*this);
int carry=0;
       unsigned __int64 sum=0;
       if(X.m_nLength<A.m_nLength)X.m_nLength=A.m_nLength;
for(int i=0;i<X.m_nLength;i++)
{
sum=A.m_ulvalue[i];
sum=sum+X.m_ulvalue[i]+carry;
X.m_ulvalue[i]=(unsigned long)sum;
if(sum>0xffffffff)carry=1;
else carry=0;
}
if(X.m_nLength<BI_MAXLEN)
{
X.m_ulvalue[X.m_nLength]=carry;
   X.m_nLength+=carry;
}
return X;
}
else{X.Mov(A);X.m_nSign=1-X.m_nSign;return Sub(X);}
}

file://�������
file://������ʽ��N.Sub(A)������ֵ��N-A
file://��������������ͬ����ֵ���������ı���������ٵ��ô�����Ӻ���
/ ****************************************************************** /
���磺
     A  B  C
-       D  E
--------------
=    F  G  H

���У���C>=E����H=C-E��carry(��λ��־)=0
     ��C<E����H=C-E+0x100000000��carry=1

     ��B-carry>=D����G=B-carry-D��carry=0      
     ��B-carry<D����G=B-carry-D+0x10000000��carry=1

     ��carry=0����F=A
     ��carry=1��A>1����F=A-1
     ��carry=1��A=1����F=0
/ ***************************************************************** /
CBigInt CBigInt::Sub(CBigInt& A)
{
CBigInt X;
if(m_nSign==A.m_nSign)
{
X.Mov(*this);
int cmp=X.Cmp(A); 
if(cmp==0){X.Mov(0);return X;}
int len,carry=0;
unsigned __int64 num;
unsigned long *s,*d;
       if(cmp>0)
               {
                       s=X.m_ulvalue;
                       d=A.m_ulvalue;
                       len=X.m_nLength;
               }
       if(cmp<0)
               { 
                       s=A.m_ulvalue;
                       d=X.m_ulvalue;
                       len=A.m_nLength;
                       X.m_nSign=1-X.m_nSign;
               }
       for(int i=0;i<len;i++)
{
if((s[i]-carry)>=d[i])
{
X.m_ulvalue[i]=s[i]-carry-d[i];
carry=0;
}
else
{
num=0x100000000+s[i];
X.m_ulvalue[i]=(unsigned long)(num-carry-d[i]);
carry=1;
}
}
while(X.m_ulvalue[len-1]==0)len--;
X.m_nLength=len;
return X;
}
else{X.Mov(A);X.m_nSign=1-X.m_nSign;return Add(X);}
}

file://�������
file://������ʽ��N.Mul(A)������ֵ��N*A
/ ****************************************************************** /
���磺
        A  B  C
*          D  E
----------------
=    S  F  G  H
+ T  I  J  K
----------------
= U  V  L  M  N

���У�SFGH=ABC*E��TIJK=ABC*D

�����ڣ�
     A  B  C
*          E
-------------
= S  F  G  H   

���У���C*E<=0xffffffff����H=C*E��carry(��λ��־)=0
     ��C*E>0xffffffff����H=(C*E)&0xffffffff
       carry=(C*E)/0xffffffff
     ��B*E+carry<=0xffffffff����G=B*E+carry��carry=0
     ��B*E+carry>0xffffffff����G=(B*E+carry)&0xffffffff
       carry=(B*E+carry)/0xffffffff
     ��A*E+carry<=0xffffffff����F=A*E+carry��carry=0
     ��A*E+carry>0xffffffff����F=(A*E+carry)&0xffffffff
       carry=(A*E+carry)/0xffffffff
     S=carry
/ ***************************************************************** /
CBigInt CBigInt::Mul(CBigInt& A)
{
CBigInt X,Y;
unsigned __int64 mul;
       unsigned long carry;
       for(int i=0;i<A.m_nLength;i++)
{
Y.m_nLength=m_nLength;
carry=0;
for(int j=0;j<m_nLength;j++)
{
mul=m_ulvalue[j];
mul=mul*A.m_ulvalue[i]+carry;
Y.m_ulvalue[j]=(unsigned long)mul;
carry=(unsigned long)(mul>>32);
}
if(carry&&(Y.m_nLength<BI_MAXLEN))
               {
                       Y.m_nLength++;
                       Y.m_ulvalue[Y.m_nLength-1]=carry;
               }
if(Y.m_nLength<BI_MAXLEN-i)
{
Y.m_nLength+=i;
       for(int k=Y.m_nLength-1;k>=i;k--)Y.m_ulvalue[k]=Y.m_ulvalue[k-i];
       for(k=0;k<i;k++)Y.m_ulvalue[k]=0;
}
X.Mov(X.Add(Y));
}
if(m_nSign+A.m_nSign==1)X.m_nSign=0;
else X.m_nSign=1;
return X;
}

file://�������
file://������ʽ��N.Div(A)������ֵ��N/A
file://�����Ĺؼ����ڡ����̡���Ȼ��ͱ���˳˷��ͼ���
file://���ｫ�����������������ת�����˱��������λ��������λ������
CBigInt CBigInt::Div(CBigInt& A)
{
CBigInt X,Y,Z;
int len;
unsigned __int64 num,div;
unsigned long carry=0;
Y.Mov(*this);
while(Y.Cmp(A)>0)
{       
if(Y.m_ulvalue[Y.m_nLength-1]>A.m_ulvalue[A.m_nLength-1])
{
len=Y.m_nLength-A.m_nLength;
div=Y.m_ulvalue[Y.m_nLength-1]/(A.m_ulvalue[A.m_nLength-1]+1);
}
else if(Y.m_nLength>A.m_nLength)
{
len=Y.m_nLength-A.m_nLength-1;
num=Y.m_ulvalue[Y.m_nLength-1];
num=(num<<32)+Y.m_ulvalue[Y.m_nLength-2];
if(A.m_ulvalue[A.m_nLength-1]==0xffffffff)div=(num>>32);
else div=num/(A.m_ulvalue[A.m_nLength-1]+1);
}
else
{
                       X.Mov(X.Add(1));
break;
}
               Z.Mov(div);
Z.m_nLength+=len;
for(int i=Z.m_nLength-1;i>=len;i--)Z.m_ulvalue[i]=Z.m_ulvalue[i-len];
for(i=0;i<len;i++)Z.m_ulvalue[i]=0;
X.Mov(X.Add(Z));
Z.Mov(Z.Mul(A));
Y.Mov(Y.Sub(Z));
}
if(Y.Cmp(A)==0)X.Mov(X.Add(1));
if(m_nSign+A.m_nSign==1)X.m_nSign=0;
else X.m_nSign=1;
return X;
}

file://������ģ
file://������ʽ��N.Mod(A)������ֵ��N%A
file://��ģ������ԭ����ͬ
CBigInt CBigInt::Mod(CBigInt& A)
{
CBigInt X,Y;
int len;
unsigned __int64 num,div;
unsigned long carry=0;
X.Mov(*this);
while(X.Cmp(A)>0)
{       
if(X.m_ulvalue[X.m_nLength-1]>A.m_ulvalue[A.m_nLength-1])
{
len=X.m_nLength-A.m_nLength;
div=X.m_ulvalue[X.m_nLength-1]/(A.m_ulvalue[A.m_nLength-1]+1);
}
else if(X.m_nLength>A.m_nLength)
{
len=X.m_nLength-A.m_nLength-1;
num=X.m_ulvalue[X.m_nLength-1];
num=(num<<32)+X.m_ulvalue[X.m_nLength-2];
if(A.m_ulvalue[A.m_nLength-1]==0xffffffff)div=(num>>32);
else div=num/(A.m_ulvalue[A.m_nLength-1]+1);
}
else
{
X.Mov(X.Sub(A));
break;
}
               Y.Mov(div);
Y.Mov(Y.Mul(A));
Y.m_nLength+=len;
for(int i=Y.m_nLength-1;i>=len;i--)Y.m_ulvalue[i]=Y.m_ulvalue[i-len];
for(i=0;i<len;i++)Y.m_ulvalue[i]=0;
X.Mov(X.Sub(Y));
}
if(X.Cmp(A)==0)X.Mov(0);
return X;
}


file://��ʱֻ������ʮ�����ַ�����ת��
int CBigInt::InPutFromStr(CString& str, const unsigned int system=DEC)
{
       int len=str.GetLength();
Mov(0);
for(int i=0;i<len;i++)
       {
             Mov(Mul(system));
int k=str[i]-48;
Mov(Add(k));
 }
 return 0;
}

file://��ʱֻ������ʮ�����ַ�����ת��
int CBigInt::OutPutToStr(CString& str, const unsigned int system=DEC)
{
str="";
char ch;
CBigInt X;
X.Mov(*this);
while(X.m_ulvalue[X.m_nLength-1]>0)
{
ch=X.Mod(system)+48;
str.Insert(0,ch);
       X.Mov(X.Div(system));
}
return 0;
}

file://ŷ������㷨��Y=X.Euc(A)��ʹ���㣺YX mod A=1
file://�൱�ڶԲ�������ax-by=1����С������
file://ʵ���Ͼ��ǳ���ѧ����շת�����
/ ******************************************************************** /
���磺11x-49y=1����x

           11 x  -  49 y  =   1      a)
49%11=5 ->  11 x  -   5 y  =   1      b)
11%5 =1 ->     x  -   5 y  =   1      c)

��y=1  ����c)ʽ  ��x=6
��x=6  ����b)ʽ  ��y=13
��y=13 ����a)ʽ  ��x=58  
/ ******************************************************************** /
CBigInt CBigInt::Euc(CBigInt& A)
{
CBigInt X,Y;
X.Mov(*this);
Y.Mov(A);
if((X.m_nLength==1)&&(X.m_ulvalue[0]==1))return X;
if((Y.m_nLength==1)&&(Y.m_ulvalue[0]==1)){X.Mov(X.Sub(1));return X;}
if(X.Cmp(Y)==1)X.Mov(X.Mod(Y));
else Y.Mov(Y.Mod(X));
X.Mov(X.Euc(Y));
       Y.Mov(*this);
if(Y.Cmp(A)==1)
{
X.Mov(X.Mul(Y));
X.Mov(X.Sub(1));
X.Mov(X.Div(A));
}
else
{
X.Mov(X.Mul(A));
X.Mov(X.Add(1));
X.Mov(X.Div(Y));
}
return X;
}

file://�ɸ������㷨��Y=X.Mon(A,B)��ʹ���㣺X^A mod B=Y
file://�����ƾ��Ǹ���ѧ���ķ���ƽ����
CBigInt CBigInt::Mon(CBigInt& A, CBigInt& B)
{
CBigInt X,Y,Z;
X.Mov(1);
Y.Mov(*this);
       Z.Mov(A);
while((Z.m_nLength!=1)||Z.m_ulvalue[0])
{
if(Z.m_ulvalue[0]&1)
{
Z.Mov(Z.Sub(1));
X.Mov(X.Mul(Y));
X.Mov(X.Mod(B));
}
else
{
Z.Mov(Z.Div(2));
Y.Mov(Y.Mul(Y));
Y.Mov(Y.Mod(B));
}
}
       return X;
}


�����Ҫ˵��������Ϊ��VC�������һ��__int64���Ϳ���
���������λ���λֵ�����Խ���������0x100000000����
���������ǿ��ܵģ�������������ϵͳ�����������64λ
���Σ�����Բ���0x40000000���ƣ�������0x40000000
�����У����κ����������֡������������㣬�������
0x3fffffff*03fffffff֮�䣬С��0xffffffff����������
һ��32λ�޷�����������ʾ����ʵ�ϡ�������Ե�����õ�
freelip����������������0x40000000��������ʾ�����ģ�
�����䷴���������ֵ���ڴ��б��ֳ�����Щ����֡���




////////////////////////////////////////////////////////////////////////
// BigInt.cpp: implementation of the BigInt class.
//
//////////////////////////////////////////////////////////////////////
/ *****************************************************************
���������Դ�ļ���BigInt.cpp
˵����������MFC��1024λRSA����
***************************************************************** /
#include "stdafx.h"
#include "BigInt.h"
//С������
const static int PrimeTable[550]=
{   3,    5,    7,    11,   13,   17,   19,   23,   29,   31,
    37,   41,   43,   47,   53,   59,   61,   67,   71,   73,
    79,   83,   89,   97,   101,  103,  107,  109,  113,  127, 
    131,  137,  139,  149,  151,  157,  163,  167,  173,  179, 
    181,  191,  193,  197,  199,  211,  223,  227,  229,  233, 
    239,  241,  251,  257,  263,  269,  271,  277,  281,  283, 
    293,  307,  311,  313,  317,  331,  337,  347,  349,  353, 
    359,  367,  373,  379,  383,  389,  397,  401,  409,  419, 
    421,  431,  433,  439,  443,  449,  457,  461,  463,  467, 
    479,  487,  491,  499,  503,  509,  521,  523,  541,  547, 
    557,  563,  569,  571,  577,  587,  593,  599,  601,  607, 
    613,  617,  619,  631,  641,  643,  647,  653,  659,  661, 
    673,  677,  683,  691,  701,  709,  719,  727,  733,  739, 
    743,  751,  757,  761,  769,  773,  787,  797,  809,  811, 
    821,  823,  827,  829,  839,  853,  857,  859,  863,  877,
    881,  883,  887,  907,  911,  919,  929,  937,  941,  947, 
    953,  967,  971,  977,  983,  991,  997,  1009, 1013, 1019, 
    1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087,
    1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 
    1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 
    1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 
    1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381,
    1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 
    1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523,
    1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 
    1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 
    1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 
    1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 
    1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 
    1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 
    1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063,
    2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 
    2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 
    2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293,
    2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371,
    2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 
    2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 
    2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 
    2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 
    2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 
    2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 
    2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909,
    2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001,
    3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083,
    3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 
    3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 
    3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343,
    3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 
    3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 
    3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581,
    3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 
    3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 
    3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 
    3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 
    3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001
};
//����������󲢳�ʼ��Ϊ��
CBigInt::CBigInt()
{
    m_nLength=1;
    for(int i=0;i<BI_MAXLEN;i++)m_ulValue[i]=0;
}
//�⹹��������
CBigInt::~CBigInt()
{
}
/ ****************************************************************************************
�����Ƚ�
���÷�ʽ��N.Cmp(A)
����ֵ����N<A����-1����N=A����0����N>A����1
**************************************************************************************** /
int CBigInt::Cmp(CBigInt& A)
{
    if(m_nLength>A.m_nLength)return 1;
    if(m_nLength<A.m_nLength)return -1;
    for(int i=m_nLength-1;i>=0;i--)
    {
        if(m_ulValue[i]>A.m_ulValue[i])return 1;
        if(m_ulValue[i]<A.m_ulValue[i])return -1;
    }
    return 0;
}
/ ****************************************************************************************
������ֵ
���÷�ʽ��N.Mov(A)
����ֵ���ޣ�N����ֵΪA
**************************************************************************************** /
void CBigInt::Mov(CBigInt& A)
{
    m_nLength=A.m_nLength;
    for(int i=0;i<BI_MAXLEN;i++)m_ulValue[i]=A.m_ulValue[i];
}
void CBigInt::Mov(unsigned __int64 A)
{
    if(A>0xffffffff)
    {
        m_nLength=2;
        m_ulValue[1]=(unsigned long)(A>>32);
        m_ulValue[0]=(unsigned long)A;
    }
    else
    {
        m_nLength=1;
        m_ulValue[0]=(unsigned long)A;
    }
    for(int i=m_nLength;i<BI_MAXLEN;i++)m_ulValue[i]=0;
}
/ ****************************************************************************************
�������
������ʽ��N.Add(A)
����ֵ��N+A
**************************************************************************************** /
CBigInt CBigInt::Add(CBigInt& A)
{
    CBigInt X;
    X.Mov(*this);//X����ֵΪN
    unsigned carry=0;
    unsigned __int64 sum=0;
    if(X.m_nLength<A.m_nLength)X.m_nLength=A.m_nLength;
    for(unsigned i=0;i<X.m_nLength;i++)
    {
        sum=A.m_ulValue[i];
  sum=sum+X.m_ulValue[i]+carry;
        X.m_ulValue[i]=(unsigned long)sum;
        carry=(unsigned)(sum>>32);//�ж��Ƿ��н�λ���н�λ��carry=1,����Ϊ0��
    }
    X.m_ulValue[X.m_nLength]=carry;
    X.m_nLength+=carry;
    return X;
}
CBigInt CBigInt::Add(unsigned long A)
{
    CBigInt X;
    X.Mov(*this);
    unsigned __int64 sum;
    sum=X.m_ulValue[0];
 sum+=A;
    X.m_ulValue[0]=(unsigned long)sum;
    if(sum>0xffffffff)
    {
        unsigned i=1;
        while(X.m_ulValue[i]==0xffffffff){X.m_ulValue[i]=0;i++;}
        X.m_ulValue[i]++;
        if(m_nLength==i)m_nLength++;
    }
    return X;
}
/ ****************************************************************************************
�������
������ʽ��N.Sub(A)
����ֵ��N-A
**************************************************************************************** /
CBigInt CBigInt::Sub(CBigInt& A)
{
    CBigInt X;
    X.Mov(*this);
    if(X.Cmp(A)<=0){X.Mov(0);return X;}//���N��AС�Ļ��ͷ���0��
    unsigned carry=0;
    unsigned __int64 num;
 unsigned i;
    for(i=0;i<m_nLength;i++)
    {
        if((m_ulValue[i]>A.m_ulValue[i])||((m_ulValue[i]==A.m_ulValue[i])&&(carry==0)))
        {
            X.m_ulValue[i]=m_ulValue[i]-carry-A.m_ulValue[i];
            carry=0;
        }
        else
        {
            num=0x100000000+m_ulValue[i];
            X.m_ulValue[i]=(unsigned long)(num-carry-A.m_ulValue[i]);
            carry=1;
        }
    }
    while(X.m_ulValue[X.m_nLength-1]==0)X.m_nLength--;
    return X;
}
CBigInt CBigInt::Sub(unsigned long A)
{
    CBigInt X;
    X.Mov(*this);
    if(X.m_ulValue[0]>=A){X.m_ulValue[0]-=A;return X;}
    if(X.m_nLength==1){X.Mov(0);return X;}
    unsigned __int64 num=0x100000000+X.m_ulValue[0];
    X.m_ulValue[0]=(unsigned long)(num-A);
    int i=1;
    while(X.m_ulValue[i]==0){X.m_ulValue[i]=0xffffffff;i++;}
    X.m_ulValue[i]--;//��λ���㣻
    if(X.m_ulValue[i]==0)X.m_nLength--;
    return X;
}
/ ****************************************************************************************
�������
������ʽ��N.Mul(A)
����ֵ��N*A
**************************************************************************************** /
CBigInt CBigInt::Mul(CBigInt& A)
{
    if(A.m_nLength==1)return Mul(A.m_ulValue[0]);//�������Ϊһ�������غ���
 CBigInt X;
 unsigned __int64 sum,mul=0,carry=0;
 unsigned i,j;
 X.m_nLength=m_nLength+A.m_nLength-1; 
    for(i=0;i<X.m_nLength;i++)
 {
  sum=carry;
  carry=0;
  for(j=0;j<A.m_nLength;j++)
  {
            if(((i-j)>=0)&&((i-j)<m_nLength))
   {
    mul=m_ulValue[i-j];
    mul*=A.m_ulValue[j];
       carry+=mul>>32;
    mul=mul&0xffffffff;
    sum+=mul;
   }
        }
  carry+=sum>>32;
  X.m_ulValue[i]=(unsigned long)sum;
 }
 if(carry){X.m_nLength++;X.m_ulValue[X.m_nLength-1]=(unsigned long)carry;}
    return X;
}
CBigInt CBigInt::Mul(unsigned long A)
{
    CBigInt X;
    unsigned __int64 mul;
    unsigned long carry=0;
    X.Mov(*this);
    for(unsigned i=0;i<m_nLength;i++)
    {
        mul=m_ulValue[i];
        mul=mul*A+carry;
        X.m_ulValue[i]=(unsigned long)mul;
        carry=(unsigned long)(mul>>32);//��λ
    }
    if(carry){X.m_nLength++;X.m_ulValue[X.m_nLength-1]=carry;}
    return X;
}
/ ****************************************************************************************
�������
������ʽ��N.Div(A)
����ֵ��N/A
**************************************************************************************** /
CBigInt CBigInt::Div(CBigInt& A)
{
    if(A.m_nLength==1)return Div(A.m_ulValue[0]);
    CBigInt X,Y,Z;
    unsigned i,len;
    unsigned __int64 num,div;
    Y.Mov(*this);
    while(Y.Cmp(A)>=0)
    {       
  div=Y.m_ulValue[Y.m_nLength-1];
  num=A.m_ulValue[A.m_nLength-1];
  len=Y.m_nLength-A.m_nLength;
  if((div==num)&&(len==0)){X.Mov(X.Add(1));break;}
  if((div<=num)&&len){len--;div=(div<<32)+Y.m_ulValue[Y.m_nLength-2];}
  div=div/(num+1);
  Z.Mov(div);
  if(len)
  {
   Z.m_nLength+=len;
   for(i=Z.m_nLength-1;i>=len;i--)Z.m_ulValue[i]=Z.m_ulValue[i-len];
   for(i=0;i<len;i++)Z.m_ulValue[i]=0;
  }
  X.Mov(X.Add(Z));
        Y.Mov(Y.Sub(A.Mul(Z)));
    }
    return X;
}
CBigInt CBigInt::Div(unsigned long A)
{
    CBigInt X;
    X.Mov(*this);
    if(X.m_nLength==1){X.m_ulValue[0]=X.m_ulValue[0]/A;return X;}
    unsigned __int64 div,mul;
    unsigned long carry=0;
    for(int i=X.m_nLength-1;i>=0;i--)
    {
        div=carry;
        div=(div<<32)+X.m_ulValue[i];
        X.m_ulValue[i]=(unsigned long)(div/A);
        mul=(div/A)*A;
        carry=(unsigned long)(div-mul);
    }
    if(X.m_ulValue[X.m_nLength-1]==0)X.m_nLength--;
    return X;
}
/ ****************************************************************************************
������ģ
������ʽ��N.Mod(A)
����ֵ��N%A
**************************************************************************************** /
CBigInt CBigInt::Mod(CBigInt& A)
{
    CBigInt X,Y;
 unsigned __int64 div,num;
    unsigned long carry=0;
 unsigned i,len;
    X.Mov(*this);
    while(X.Cmp(A)>=0)
    {
  div=X.m_ulValue[X.m_nLength-1];
  num=A.m_ulValue[A.m_nLength-1];
  len=X.m_nLength-A.m_nLength;
  if((div==num)&&(len==0)){X.Mov(X.Sub(A));break;}
  if((div<=num)&&len){len--;div=(div<<32)+X.m_ulValue[X.m_nLength-2];}
  div=div/(num+1);
  Y.Mov(div);
  Y.Mov(A.Mul(Y));
  if(len)
  {
   Y.m_nLength+=len;
   for(i=Y.m_nLength-1;i>=len;i--)Y.m_ulValue[i]=Y.m_ulValue[i-len];
   for(i=0;i<len;i++)Y.m_ulValue[i]=0;
  }
        X.Mov(X.Sub(Y));
    }
    return X;
}
unsigned long CBigInt::Mod(unsigned long A)
{
    if(m_nLength==1)return(m_ulValue[0]%A);
    unsigned __int64 div;
    unsigned long carry=0;
    for(int i=m_nLength-1;i>=0;i--)
    {
        div=m_ulValue[i];
  div+=carry*0x100000000;
        carry=(unsigned long)(div%A);
    }
    return carry;
}
/ *********************************************************************
�����Լ�������÷�ʽN.Gcd(A,B);
Gcd��A��B��������ֵΪA��B�����Լ��d->N��
********************************************************************** /
void CBigInt::Gcd(CBigInt &A,CBigInt &B)
{
  CBigInt a,b,temp;
  a.Mov(A);
  b.Mov(B);
  temp.Mov(a.Mod(b));
  while(temp.m_ulValue[0]!=0)
  {  
  a.Mov(b);
     b.Mov(temp);
     temp.Mov(a.Mod(b));
  }
  (*this).Mov(b);
}
unsigned long CBigInt::Gcd(CBigInt &A,unsigned long B)
{
   CBigInt a,temp;
   a.Mov(A);
   unsigned  long b=B;
   temp.Mov(a.Mod(b));
   while(temp.m_ulValue[0]!=0)
   {
    a.Mov(b);
    b=temp.m_ulValue[0];
    temp.Mov(a.Mod(b));
   }
   return b;
}
/ ****************************************************************************************
���ַ�����10���ƻ�16���Ƹ�ʽ���뵽����
���ø�ʽ��N.Get(str,sys)
����ֵ��N����ֵΪ��Ӧ����
sys��ʱֻ��Ϊ10��16
**************************************************************************************** /
void CBigInt::Get(CString& str, unsigned int system)
{
    int len=str.GetLength(),k;
    Mov(0);
    for(int i=0;i<len;i++)
    {
       Mov(Mul(system));
       if((str[i]>='0')&&(str[i]<='9'))k=str[i]-48;
       else if((str[i]>='A')&&(str[i]<='F'))k=str[i]-55;
       else if((str[i]>='a')&&(str[i]<='f'))k=str[i]-87;
       else k=0;
       Mov(Add(k));
    }
}
/ ****************************************************************************************
��������10���ƻ�16���Ƹ�ʽ���Ϊ�ַ���
���ø�ʽ��N.Put(str,sys)
����ֵ���ޣ�����str����ֵΪN��sys�����ַ���
sys��ʱֻ��Ϊ10��16
**************************************************************************************** /
void CBigInt::Put(CString& str, unsigned int system)
{
    if((m_nLength==1)&&(m_ulValue[0]==0)){str="0";return;}
 str="";
    CString t="0123456789ABCDEF";
    int a;
    char ch;
    CBigInt X;
    X.Mov(*this);//X����ֵΪSTR��
    while(X.m_ulValue[X.m_nLength-1]>0)
    {
        a=X.Mod(system);
        ch=t[a];
        str.Insert(0,ch);
        X.Mov(X.Div(system));//X=X/system;
    }
}
/ ****************************************************************************************
�󲻶�����ax-by=1����С������
���÷�ʽ��N.Euc(A)
����ֵ��X,���㣺NX mod A=1
**************************************************************************************** /
CBigInt CBigInt::Euc(CBigInt& A)
{
 CBigInt M,E,X,Y,I,J;
    int x,y;
 M.Mov(A);
 E.Mov(*this);
 X.Mov(0);
 Y.Mov(1);
 x=y=1;
 while((E.m_nLength!=1)||(E.m_ulValue[0]!=0))
 {
  I.Mov(M.Div(E));
  J.Mov(M.Mod(E));
  M.Mov(E);
  E.Mov(J);
  J.Mov(Y);
  Y.Mov(Y.Mul(I));
  if(x==y)
  {
      if(X.Cmp(Y)>=0)Y.Mov(X.Sub(Y));
   else{Y.Mov(Y.Sub(X));y=0;}
  }
  else{Y.Mov(X.Add(Y));x=1-x;y=1-y;}
  X.Mov(J);
 }
 if(x==0)X.Mov(A.Sub(X));
 return X;
}
/ ****************************************************************************************
��˷���ģ
���÷�ʽ��N.RsaTrans(A,B)
����ֵ��X=N^A MOD B
**************************************************************************************** /
CBigInt CBigInt::RsaTrans(CBigInt& A, CBigInt& B)
{
    CBigInt X,Y;
 int i,j,k;
 unsigned n;
 unsigned long num;
 k=A.m_nLength*32-32;
 num=A.m_ulValue[A.m_nLength-1];
 while(num){num=num>>1;k++;}
 X.Mov(*this);
 for(i=k-2;i>=0;i--)
 {
  Y.Mov(X.Mul(X.m_ulValue[X.m_nLength-1]));
  Y.Mov(Y.Mod(B));
        for(n=1;n<X.m_nLength;n++)
  {          
   for(j=Y.m_nLength;j>0;j--)Y.m_ulValue[j]=Y.m_ulValue[j-1];
   Y.m_ulValue[0]=0;
   Y.m_nLength++;
   Y.Mov(Y.Add(X.Mul(X.m_ulValue[X.m_nLength-n-1])));
   Y.Mov(Y.Mod(B));
  }
  X.Mov(Y);
  if((A.m_ulValue[i>>5]>>(i&31))&1)
  {
      Y.Mov(Mul(X.m_ulValue[X.m_nLength-1]));
      Y.Mov(Y.Mod(B));
            for(n=1;n<X.m_nLength;n++)
   {          
       for(j=Y.m_nLength;j>0;j--)Y.m_ulValue[j]=Y.m_ulValue[j-1];
       Y.m_ulValue[0]=0;
       Y.m_nLength++;
       Y.Mov(Y.Add(Mul(X.m_ulValue[X.m_nLength-n-1])));
       Y.Mov(Y.Mod(B));
   }
      X.Mov(Y);
  }
 }
    return X;
}
/ ****************************************************************************************
���������㷨��������
���÷�ʽ��N.Rab()
����ֵ����NΪ����������1�����򷵻�0
**************************************************************************************** /
int CBigInt::Rab()
{
    unsigned i,j,pass;
    for(i=0;i<550;i++){if(Mod(PrimeTable[i])==0)return 0;}
    CBigInt S,A,I,K;
    K.Mov(*this);
 K.m_ulValue[0]--;
    for(i=0;i<5;i++)
    {
        pass=0;
        A.Mov(rand()*rand());
  S.Mov(K);
        while((S.m_ulValue[0]&1)==0)
  {
            for(j=0;j<S.m_nLength;j++)
   {
       S.m_ulValue[j]=S.m_ulValue[j]>>1;
       if(S.m_ulValue[j+1]&1)S.m_ulValue[j]=S.m_ulValue[j]|0x80000000;
   }
      if(S.m_ulValue[S.m_nLength-1]==0)S.m_nLength--;
   I.Mov(A.RsaTrans(S,*this));
   if(I.Cmp(K)==0){pass=1;break;}
  }
  if((I.m_nLength==1)&&(I.m_ulValue[0]==1))pass=1;
  if(pass==0)return 0;
 }
    return 1;
}
/ ****************************************************************************************
�����������
���÷�����N.GetPrime(bits)
����ֵ��N����ֵΪһ��bitsλ��0x100000000���Ƴ��ȣ�������
**************************************************************************************** /
void CBigInt::GetPrime(int bits)
{
    unsigned i;
    m_nLength=bits;
begin:
 for(i=0;i<m_nLength;i++)m_ulValue[i]=rand()*0x10000+rand();
    m_ulValue[0]=m_ulValue[0]|1;
 for(i=m_nLength-1;i>0;i--)
 {
  m_ulValue[i]=m_ulValue[i]<<1;
  if(m_ulValue[i-1]&0x80000000)m_ulValue[i]++;
 }
 m_ulValue[0]=m_ulValue[0]<<1;
 m_ulValue[0]++;
    for(i=0;i<550;i++){if(Mod(PrimeTable[i])==0)goto begin;}
    CBigInt S,A,I,K;
    K.Mov(*this);
 K.m_ulValue[0]--;
    for(i=0;i<5;i++)
 {
        A.Mov(rand()*rand());
     S.Mov(K.Div(2));
     I.Mov(A.RsaTrans(S,*this));
     if(((I.m_nLength!=1)||(I.m_ulValue[0]!=1))&&(I.Cmp(K)!=0))goto begin;
 }
}
/ ****************************************************************
ʵ�ֿ���ģ�˵�Montgomery�㷨�����ͷ���ƽ������Ͽ���ʵ�ִ����Ŀ���
ģ�ˡ�������ʽ��N.(A,B,R,N)
***************************************************************** /
/ ****************************************************************************************
�󲻶�����ax-by=1����С������
���÷�ʽ��N.Euc(A)
����ֵ��X,���㣺NX mod A=1
**************************************************************************************** /
void CBigInt::Mon(CBigInt A,CBigInt B,unsigned long R,CBigInt N)
{
    CBigInt R_1,n;
 n.Mov(N);
 unsigned long temp,N_1;
 //����һ�������N���ص���������
   do{
       R=rand();//������������Ҽ����Ƿ������N���أ�
        temp=Gcd(N,R);
   }while(temp!=1);
   //���R*(R��ģN��)=1Mod(N);�Ľ⣻����R��ģN�棻
   R_1.Mov(R);
   R_1.Euc(N);
   N_1=1;
   while(N_1<=R)
   {
       n.Mul(N_1);
    n.Mod(R);
    if(n.m_ulValue[0]==(R-1))break;
    N_1++;
   }
      
}


////////////////////////////////////////////////////////////
#include "Big__CALC.h"  
  
int main()  
{  
    char str1[200]="9876543210987";     
    char str2[200]="1234567890";       
    char result[200]={0};  
  
    printf("str1        = %s\t%d\n",str1,strlen(str1)-1);  
    printf("str2        = %s\t%d\n",str2,strlen(str2)-1);  
  
    int ret = Compare(str1,str2);  
    if (ret==1)  
    {  
        printf("str1 > str2\n");  
    }  
    else if(ret==-1) printf("str1 < str2\n");  
    else printf("str1 = str2\n");  
      
    Add(str1,str2,result);  
    printf("str1 + str2 = %s\t%d\n",result,strlen(result)-1);  
      
  
    Sub(str1,str2,result);  
    printf("str1 - str2 = %s\t%d\n",result,strlen(result)-1);  
      
  
    Chen(str1,str2,result);  
    printf("str1 * str2 = %s\t%d\n",result,strlen(result)-1);  
  
    char div_result[200]={0};  
    Mod(str1,str2,result,div_result);  
    printf("str1 M str2 = %s\t%d\n",result,strlen(result)-1);  
    printf("str1 / str2 = %s\t%d\n",div_result,strlen(div_result)-1);  
       
    int num=10;  
    char str[200]="999";  
    ChenFang(str,num,result);  
    ThrowAway_0(str);  
    printf("%s ^ %d = %s\t%d\n",str,num,result,strlen(result)-1);  
  
    getchar();  
  
    return 1;  
}  
  
  
void Trans(char *str_num1, char *str_num2, char *tempbuf1, char *tempbuf2)  
{  
    int len_num1=0;  
    int len_num2=0;  
    int i=0;  
    while(str_num1[i]!='\0')  
    {  
        len_num1++;  
        i++;  
    }     
//  printf("�ַ���1�ĳ���: length1=%d\n",len_num1);  
    i=0;  
    while(str_num2[i]!='\0')  
    {  
        len_num2++;  
        i++;  
    }     
//  printf("�ַ���2�ĳ���: length2=%d\n\n",len_num2);  
  
    tempbuf1[0]='0';  
    tempbuf2[0]='0';  
  
//=======================================================================  
    if(len_num2>=len_num1)                                   //������ͬ����  
    {  
        for(i=1;i<=(len_num2-len_num1);i++)  
        {  
            tempbuf1[i]='0';  
        }  
        for(i=len_num2-len_num1+1;i<=len_num2;i++)  
        {  
            tempbuf1[i]=str_num1[i-(len_num2-len_num1+1)];  
        }  
        for(i=1;i<=len_num2;i++)  
        {  
            tempbuf2[i]=str_num2[i-1];  
        }  
    }  
//------------------------------------------  
    else if(len_num2<len_num1)  
    {  
        for(i=1;i<=(len_num1-len_num2);i++)  
        {  
            tempbuf2[i]='0';  
        }  
        for(i=len_num1-len_num2+1;i<=len_num1;i++)  
        {  
            tempbuf2[i]=str_num2[i-(len_num1-len_num2+1)];  
        }  
        for(i=1;i<=len_num1;i++)  
        {  
            tempbuf1[i]=str_num1[i-1];  
        }  
    }  
/ *  printf("-------------------------------------------------------\nת��֮��:\n"); 
    printf("str1=: %s\n",tempbuf1); 
    printf("str2=: %s\n",tempbuf2); 
//========================================================================== 
    int len=0; 
    i=0; 
    while(tempbuf1[i]!='\0') 
    { 
        len++; 
        i++; 
    } 
* /  
}  
  
//====================================================================================================  
//extern "C" __declspec(dllexport)  
void  Chen(char *tempbuf1,   char *tempbuf2 , char *result)  
{  
    char buf1[200]={0};  
    char buf2[200]={0};  
    Trans(tempbuf1,tempbuf2,buf1,buf2);  
    strcpy(tempbuf1,buf1);  
    strcpy(tempbuf2,buf2);  
  
    int len=0;  
    int i=0;  
    int j=0;  
    int n=0;  
    int jinwei=0;  
  
    while(tempbuf1[i]!='\0')  // �ַ�������  
    {  
        len++;  
        i++;  
    }  
  
    int temp[200]={0};        // �����������洢���η���ϵ�� 10Ϊ��  
  
    int max=2*len;  
    for(i=0;i<=max;i++)  
    {  
        if(i<len-1)  
        {  
            for(j=0;(j<=len-1)&&(j>=i+1-len)&&(j<=i);j++)  
            {             
                temp[i]+=((int)tempbuf1[len-1-j]-48 )*((int)tempbuf2[len-1-i+j]-48);              
            }  
            temp[i]+=jinwei;  
              
            if (temp[i]>=10)  //&&temp[i]<100  
            {  
                jinwei  = temp[i]/10;  
                temp[i] = temp[i]%10;  
            }  
            else jinwei=0;  
        }  
        else if (i>=len-1)  
        {  
            for(j=i-len+2;(j<=len-1)&&(j>=i+1-len)&&(j<=i);j++)  
            {             
                temp[i]+=((int)tempbuf1[len-1-j]-48 )*((int)tempbuf2[len-1-i+j]-48);              
            }  
            temp[i]+=jinwei;  
              
            if (temp[i]>=10)  //&&temp[i]<100  
            {  
                jinwei  = temp[i]/10;  
                temp[i] = temp[i]%10;  
            }  
            else jinwei=0;  
        }  
          
          
    }  
                                
    i=max;  
    while(i>=0)  
    {  
        if(temp[i]!=0)  
            break;  
          
        else i--;                 
    }  
//  printf("str1 * str2= ");  
    int num=i; // ��ѧ����������  
/ *  for (j=num;j>=0;j--) 
    { 
         
        printf("%d",temp[j]); 
         
    }    
    printf("\t%d\n",num); 
* /  
//===========================================  
    memset(result,0,200);  
    for(i=num;i>=0;i--)  
    {  
        result[num-i]=(char)(temp[i]+48);  
  
    }  
//  printf("result = %s\t%d\n",result,strlen(result)-1);  
      
}  
  
  
//=====================================================================  
  
//extern "C" __declspec(dllexport)   
void  ChenFang(char *tempbuf1,   int num, char *result)    // �����˷�  
{  
    static int count=0;  
  
    if (count==0)  
    {  
        char c[200]={'1',0,0};  
        memset(result,0,200);  
        Chen( tempbuf1 , c , result);  
  
    }  
    count++;  
  
    if(num==1)  
    {  
        return ;  
    }  
  
    else if(num>1)  
    {  
        Chen( tempbuf1 , result , result);  
        num--;  
        ChenFang(tempbuf1,num,result);  
    }  
  
    ThrowAway_0 (result);  
  
}  
  
  
//=================================================  
  
//extern "C" __declspec(dllexport)   
void  Mod(char *tempbuf1,   char *tempbuf2, char *result , char *div_result )    // ��������  
{  
    memset(result,0,200);  
    ThrowAway_0 (tempbuf1);  
    ThrowAway_0 (tempbuf2);  
    int max1 = strlen(tempbuf1);  
    int max2 = strlen(tempbuf2);  
    int temp=0;  
    int ret=1;  
    char div[200]={0};  
    int  count=0;  
    char AfterSub[200]={0};  
  
    if(max1-max2>=0)          
    {  
        while(ret==1)    
        {                             
            Sub (tempbuf1,tempbuf2,AfterSub);   //��ȥ֮�����ж��Ƿ�õ����----����tempbuf1 < tempbuf2  
            memset(tempbuf1,0,200);  
            strcpy (tempbuf1,AfterSub);   
            ret=Compare(tempbuf1,tempbuf2);  
            ThrowAway_0 (tempbuf1);  
            ThrowAway_0 (tempbuf2);   
            count++;  
            if(count>=1000)  
            {  
                itoa(count,div,10);  
                Add(div_result,div,div_result);  
                count=0;  
            }  
              
        }  
              
        strcpy (result,tempbuf1);  
        ThrowAway_0 (result);  
        itoa(count,div,10);  
        Add(div_result,div,div_result);  
    }     
    else  // if (Compare(tempbuf1,tempbuf2)==-1) // ���ߴ�Ļ���mod ���� tempbuf1  
    {  
        strcpy (result,tempbuf1);  
        ThrowAway_0 (result);  
    }  
}  
  
//========================================================  
  
//extern "C" __declspec(dllexport)   
void Add(char *tempbuf1,   char *tempbuf2, char *result) // ������� result = tempbuf1 + tempbuf2  
{  
    char buf1[200]={0};  
    char buf2[200]={0};  
    Trans(tempbuf1,tempbuf2,buf1,buf2);  
    strcpy(tempbuf1,buf1);  
    strcpy(tempbuf2,buf2);  
  
    int i=0;  
    int temp=0;  
    int jinwei=0;  
    int len=0;  
    while(tempbuf1[i]!='\0')  
    {  
        len++;  
        i++;  
    }  
    for(i=len-1;i>=0;i--)  
    {     
        temp=(int)(tempbuf1[i]+tempbuf2[i]+jinwei-96);  
        if(temp>=10)  
        {  
            temp=temp-10;  
            jinwei=1;  
        }  
        else jinwei=0;  
        result[i]=(char)(temp+48);    
    }  
    ThrowAway_0 (result);  
  
}  
  
//================================================  
  
void  ThrowAway_0 (char *tempbuf )  // ȥ�����ǰ��� ������������� "0"  
{  
    char buf[200]={0};  
  
    int n = strlen(tempbuf)-1;  
    int i=0;  
    while(i<n)  
    {  
        if (tempbuf[i]!='0')  
        {  
            break;  
        }  
        else   
        {  
            i++;  
        }  
    }  
    int Throw = i;  
    for (i=0;i<=n-Throw;i++)  
    {  
        buf[i]=tempbuf[i+Throw];  
    }  
  
    strcpy(tempbuf,buf);  
  
}  
  
//=======================================================  
  
//extern "C" __declspec(dllexport)   
void Sub(char *tempbuf1, char *tempbuf2, char *result)     // ������� result = tempbuf1 - tempbuf2  
{  
    ThrowAway_0 (tempbuf1);  
    ThrowAway_0 (tempbuf2);  
    memset(result,0,200);  
    char buf1[200]={0};  
    char buf2[200]={0};  
    Trans(tempbuf1,tempbuf2,buf1,buf2);  
    memset(tempbuf1,0,200);  
    memset(tempbuf2,0,200);  
  
    strcpy(tempbuf1,buf1);  
    strcpy(tempbuf2,buf2);  
  
    int i=0;  
    int temp=0;  
    int jiewei=0;  
    int len=0;  
    int ret=1;  
  
    while(tempbuf1[i]!='\0')    // tempbuf1 �� tempbuf2 �ĳ������  
    {  
        len++;  
        i++;  
    }  
      
    ret = Compare(tempbuf1,tempbuf2);  
    if(ret==1)  
    {  
        for(i=len-1;i>=0;i--)  
        {    
            temp = (int)tempbuf1[i] - (int)tempbuf2[i] - jiewei;  
            if (temp>=0)  
            {  
                result[i]=(char)(temp+48);  
                jiewei=0;  
            }  
            else if (temp<0)  
            {  
                result[i]=(char)(temp+10+48);  
                jiewei=1;  
            }  
        }  
        ThrowAway_0 (result);  
    }  
    else if(ret==0)   
    {  
        memset(result,0,200);  
        result[0]='0';  
    }  
    else if(ret==-1)   
    {  
        for(i=len-1;i>=0;i--)  
        {    
            temp = (int)tempbuf2[i] - (int)tempbuf1[i] - jiewei;  
            if (temp>=0)  
            {  
                result[i]=(char)(temp+48);  
                jiewei=0;  
            }  
            else if (temp<0)  
            {  
                result[i]=(char)(temp+10+48);  
                jiewei=1;  
            }  
        }  
        ThrowAway_0 (result);  
        memset(buf1,0,200);  
        sprintf(buf1,"-%s",result);   
        strcpy(result,buf1);  
    }  
      
}  
  
//======================================================================  
  
//===================================================================================  
  
int Compare(char *tempbuf1,char *tempbuf2)  
{  
    ThrowAway_0 (tempbuf1);  
    ThrowAway_0 (tempbuf2);  
    char buf1[200]={0};  
    char buf2[200]={0};  
    Trans(tempbuf1,tempbuf2,buf1,buf2);  
    memset(tempbuf1,0,200);  
    memset(tempbuf2,0,200);  
    strcpy(tempbuf1,buf1);  
    strcpy(tempbuf2,buf2);  
      
  
    int ret=1;  
    int count=0;      
    while(count<200)  
    {  
          
        int m=(int)tempbuf1[count]-48;  
        int n=(int)tempbuf2[count]-48;  
        if(m==n)  
        {  
            count++;  
            ret=0;  
        }  
        else if(m>n)  
        {  
        //  printf("tempbuf1>tempbuf2\n");  
            ret=1;  
            break;  
              
        }  
        else if(m<n)  
        {  
        //  printf("tempbuf1<tempbuf2\n");  
            ret=-1;  
            break;  
        }  
    }  
    return ret;  
}  
2.ͷ�ļ�

[cpp] view plain copy
#include <stdio.h>   
#include <string.h>  
#include <stdlib.h>  
  
//extern "C" __declspec(dllexport)   
void Add (char *tempbuf1,   char *tempbuf2, char *result);  // ������� result = tempbuf1 + tempbuf2  
  
//extern "C" __declspec(dllexport)   
void Sub(char *tempbuf1, char *tempbuf2, char *result);     // ������� result = tempbuf1 - tempbuf2 (Ĭ��ǰ�ߴ�)  
  
int Compare(char *tempbuf1,char *tempbuf2);  
  
//extern "C" __declspec(dllexport)    
void Chen (char *tempbuf1,   char *tempbuf2, char *result); // ������� result = tempbuf1 * tempbuf2  
  
void Trans (char *str_num1,   char *str_num2, char *tempbuf1, char *tempbuf2); // ����ת��Ϊ�ȳ�������ǰ����� ��0��  
  
  
//extern "C" __declspec(dllexport)                                              // �������� div_result = tempbuf1 / tempbuf2  
void  Mod(char *tempbuf1,   char *tempbuf2, char *result , char *div_result );    // �������� result = tempbuf1 % tempbuf2    
  
  
//extern "C" __declspec(dllexport)   
void  ChenFang (char *tempbuf1,   int num, char *result);    // �����˷�   result = tempbuf1 ^ num  
  
void  ThrowAway_0 (char *tempbuf );  // ȥ�����ǰ��� ������������� "0"  


ԭ�����

RSA ԭ��

ѡȡ������ͬ�Ĵ�����p��q��������N=p*q��ѡȡС����d��������e��ʹd*e % (p-1)(q-1)=1��
��������A<N��
��B=A**d % N
��A=B**e % N

�ɼ�d��e�γ��˷ǶԳ���Կ��ϵ���������ù�Կd���ܣ������߿���˽Կe���ܣ������߼�ʹ����������B����Կd��N���ڲ�֪��p��q��ǰ���£��޷������e���Ӷ��޷��������A����Nȡ�ǳ����ֵʱ��������ʽ�ֽ��p��q�Ƿǳ����ѵģ����統NΪ1024 bitʱ���ݷ������趯�ü�ֵ��ǧ������Ĵ��ͼ����ϵͳ���ķ�һ���ʱ�䡣

RSA ��Կ��ѡȡ�ͼӽ��ܹ��̶��ǳ���࣬���㷨����ҪҪʵ���ĸ����⣺

1����δ����������
2��������ͬ�෽�� XY % M = 1
3����ο��ٽ���ģ������
4����λ�ȡ������

ʵ���ϣ���ʵ��RSA �㷨�Ĺ����д�һᷢ�ֺ��������ⲻ�Ǹ��Զ����ģ����ǻ��й������������ף����Ž�ʱ�����ʶ����RSA�㷨��һ�֡����������㷨��

 

�����洢��

RSA �����������㣬Ŀǰ����RSA �㷨��������1024λ�Ĵ�������֮�ϡ���������ı�����ֻ��֧�ֵ�64λ���������㣬����������������ʹ�õ���������С�ڵ���64λ������0xffffffffffffffff��Ҳ����18446744073709551615����ԶԶ�ﲻ��RSA ����Ҫ��������Ҫר�Ž�������������������һ���⡣

��򵥵İ취�ǽ���������������д�������ĸ�Ԫ��Ҳ���Ǵ���ÿһλ�ϵ����֣�ͨ����������������ʮ��������0��9��Ȼ��ԡ��������顱��д�Ӽ��˳�����������������Ч�ʺܵͣ���Ϊ������Ϊ1024λ�Ĵ�����ʮ������Ҳ�����ٶ�λ�������κ�һ�����㣬����Ҫ�����������ٸ�Ԫ�ص�����ռ��϶����ѭ������Ҫ������Ŀռ��ż���Ľ���λ��־���м��������⣬����ĳЩ�����������ԣ����ö����ƻ�ʹ������̴��򻯣������ִ�����ʾ����ת���ɶ�������Ȼ�ǳ��鷳��������ĳЩʵ������ɴ�����˶���������ķ�������¼��������Ȼ����Ч�ʾ͸����ˡ�

һ����Ч�ĸĽ������ǽ�������ʾΪһ��n �������飬����Ŀǰ��32λϵͳ����n ����ȡֵΪ2 ��32�η����� 0x100000000�����罫һ��������Ϊ1024λ�Ĵ���
ת����0x10000000���ƣ��ͱ����32λ����ÿһλ��ȡֵ��Χ�����Ƕ����Ƶ�0��1��ʮ���Ƶ�0��9������0-0xffffffff���������ÿ�����һ��32λ��DWORD ���磺�޷��ų�������unsigned long�� ��������ʾ��ֵ������1024λ�Ĵ����ͱ��һ������32��Ԫ�ص� DWORD���飬����� DWORD������и������������ѭ����ģ����32�ζ��ѡ�����0x100000000 ����������ƣ����ڼ������˵��������һ���£�ת���ǳ����ס�

�������18446744073709551615������ 0xffffffff ffffffff�����൱��ʮ���Ƶ�99������λ��ÿλ����0xffffffff����18446744073709551616����0x00000001 00000000 00000000�����൱��ʮ���Ƶ�100������λ����һλ��1 ��������λ����0 ����˵ȵȡ���ʵ��Ӧ���У����������顱������˳����õ�λ��ǰ��λ�ں�ķ�ʽ������������A �Ϳ��Է��������ѧ���ʽ����ʾ��ֵ��
A=Sum[i=0 to n](A[i]*r**i)��r=0x100000000��0<=A<r
����Sum ��ʾ��ͣ�A[i]��ʾ���Լ�¼A ������ĵ�i ��Ԫ�أ�**��ʾ�˷���

�κ������������ն��ֽܷ������������֮������㣬��0x100000000 �������䡰���֡����ﵽ0xffffffff��������������֮������㣬���Ҳ��Ȼ������Ŀǰ32λϵͳ���ֳ�����VC++�У�����һ��__int64 ���Ϳ��Դ���64λ�����������Բ��õ�����һ���⣬������������ϵͳ�����������64λ���Σ�����Ҫ���ø�С��
���Ʒ�ʽ���洢����������16λ��WORD���Ϳ���������ʾ0x10000 ���ơ���Ч�ʸ��ߵİ취���ǲ���32λ�� DWORD���ͣ�ֻ������0x100000000 ���Ƹĳ�0x40000000���ƣ������������ֽ�����������������Ϊ 0x3fffffff * 0x3fffffff��С��0xffffffffffffff��������һ��˫���ȸ������ͣ�double��52λ��Ч���֣���������һ�м�����ֻ�ǲ��ܼ򵥵��ø�λ��λ�����м�����ֳ����������֡���

 

�����Ӽ��˳���

���д���A��B��C������A>=B��
A=Sum[i=0 to p](A[i]*r**i)
B=Sum[i=0 to q](B[i]*r**i)
C=Sum[i=0 to n](C[i]*r**i)
r=0x100000000(32λ)��0<=A[i],B[i],C[i]<r��p>=q(A[i],B[i],C[i]����32λ����)
��C=A+B��C=A-B��C=A*Bʱ�����Ƕ�����ͨ�������C�����C��

1.�ӷ�
C=A+B����ȻC[i]�����ǵ���A[i]+B[i]����ΪA[i]+B[i]����>0xffffffff����C[i]����<=0xffffffff����ʱ����Ҫ��λ����Ȼ�ڼ���C[i-1]ʱҲ���ܲ����˽�λ�����Լ���C[i]ʱ��Ҫ�����ϴεĽ�λֵ�������һ��64λ����result����¼�ͣ�64λ��Ϊ�˷�׼���ģ�ʵ�ʼӼ���ֻҪ33λ���ɣ�����һ��32λ����carry����¼��λ(ΪʲôҪ32λ��Ϊ�˷�׼���ģ�ʵ�ʼӼ�����λֻ��1)�����У�

carry=0;
for(i=0;i<=p;i++) ��      //i��0��p ��ΪA>B
����result=A[i]+B[i]+carry;
����C[i]=result%0x100000000 ;   //�����￴resultӦ�ô���64λ������65λ
����carry=result/0x100000000;
��
if(carry=0) n=p;
else n=p+1;

2.����
C=A-B��ͬ��C[i]�����ǵ���A[i]-B[i]����ΪA[i]-B[i]����<0����C[i]����>=0����ʱ����Ҫ��λ��ͬ���ڼ���C[i-1]ʱҲ���ܲ����˽�λ�����Լ���Cʱ��Ҫ��ȥ�ϴεĽ�λֵ��

carry=0
for(i=0;i<=p;i++) {     //i��0��p ��ΪA>B
����if((A[i]-B[i]-carry)>=0){
��������C[i]=A[i]-B[i]-carry;
��������carry=0;
����}
����else{
��������C[i]=0x100000000+A[i]-B[i]-carry;
��������carry=1;
����}
}
n=p;
while (C[n]==0) n=n-1;   //��ǰ�ߵ�0ȥ��

3.�˷�
C=A*B������������Ҫ�۲��ճ����˷����õġ���ʽ���㡱���̣�

������������  ��������������A3 A2 A1 A0
������������    ��������������* B2 B1 B0
------------------------------------------
���������� ����= A3B0 A2B0 A1B0 A0B0
�������� + A3B1 A2B1 A1B1 A0B1
    + A3B2 A2B2 A1B2 A0B2
------------------------------------------
=        C5     C4     C3    C2    C1     C0

���Թ��ɳ���C[i]=Sum[j=0 to q](A[i-j]*B[j]) (ע����C[i])������i-j����>=0��<=p��
��Ȼ��һ����û�п��ǽ�λ����Ȼ����A[i-j]*B[j]��Sum��ʱ�򶼿��ܷ�����λ����Ȼ������ԭ������Ľ�λ�����ۼӳ�һ����λֵ�����տ��������㷨��ɳ˷���
C = Sum[i= 0 to n](C[i]*r**i) =  Sum[i= 0 to n] ( Sum[j=0 to q](A[i-j]*B[j])  *r**i).(�����n=p+q-1��������nλ�������н�λʱnӦ��1)
CҲ���Ա�ʾ�� C= Sum[i= 0 to q](A*B[i] *r**i)

n=p+q-1
carry=0
for(i=0;i<=n;i++){
����result=carry;
����for(j=0;j<=q;j++){
��������if (0<=i-j<=p ){
������������result=result+A[i-j]*B[j];
������������C[i]=result%0x100000000;
������������carry=result/0x100000000;
��������}

����}

}
if(carry!=0) {
����n=n+1;
����C[n]=carry
} 

4.����
����C=A/B�������޷���B ��A�����̡�������ֻ��ת����B[q]��A[p]���������õ�һ������ֵ�������޷�ֱ��ͨ������C�����C��ֻ��һ�����ƽ�C�����ڣ�
B*(A[p]/B[q]-1)*0x100000000**(p-q)<C

�X=0���ظ�A=A-X*B��X=X+(A[p]/B[q]-1)*0x100000000**(p-q)��(Ϊʲô��)  ֱ��A<B
��X=A/B���Ҵ�ʱ��A=A%B

ע������������A*0x100000000**k�����ȼ��ڽ�A �������еĸ�Ԫ������k λ��
���ؼ��㣻ͬ����A/0x100000000**k��ȼ������ơ�

 

ŷ����÷��̣�

��RSA �㷨�У�����Ҫ����֪A��N������£��� B��ʹ�� (A*B)%N=1�����൱�����B��M����δ֪���Ķ�Ԫһ�β������� A*B-N*M=1 ����С�����⡣
����Բ�������ax-by=c ����С�����⣬�Ž����ⶼ���й��꾡���о���������������ŷ������㷨����շת��������й����ؾ��صġ�������һ��������ʵ�϶�Ԫһ�β���������������ĳ�Ҫ������cΪa��b�����Լ��������c=1ʱ��a��b���뻥�ʡ�����RSA�㷨�����ڹ�ԿdΪ�������������κ����������ʣ����Կ���ͨ
��ŷ����·��������˽Կe��

ŷ������㷨��һ�ֵݹ��㷨���Ƚ�������⣺

���磺11x-49y=1����x
��a�� 11 x - 49 y = 1 49%11=5 ->
��b�� 11 x - 5 y = 1 11%5 =1 ->
��c�� x - 5 y = 1
��y=0 ���루c����x=1
��x=1 ���루b����y=2
��y=2 ���루a����x=9

ͬ���ʹ�õݹ��㷨������� ax-by=1��a��b���ʣ��Ľ⡣ʵ����ͨ���������ɽ��ݹ��㷨ת���ɷǵݹ��㷨�ͱ���˴�����һ����

x=0,y=1
WHILE a!=0
i=y
y=x-y*a/b
x=i
i=a
a=b%a
b=i
IF x<0 x=x+b
RETURN x

 

ģ������

ģ��������RSA �ĺ����㷨����ֱ�ӵؾ�����RSA �㷨�����ܡ���Կ���ģ��������һ���⣬�����ִ���ѧ������˴����Ľ��������ͨ�������Ƚ���ģ����ת��Ϊ��ģ���㡣

������D=C**15 % N�����ڣ�a*b % n = (a % n)*(b % n) % n�����ԣ�

C1 =C*C % N =C**2 % N
C2 =C1*C % N =C**3 % N
C3 =C2*C2 % N =C**6 % N
C4 =C3*C % N =C**7 % N
C5 =C4*C4 % N =C**14 % N
C6 =C5*C % N =C**15 % N

��������E=15����ģ����ɷֽ�Ϊ6 ����ģ���㣬���ɷ������Ϸ������Է��ֶ�������E�����ɲ��������㷨����D=C**E % N��

D=1
WHILE E>=0
IF E%2=0
C=C*C % N
E=E/2
ELSE
D=D*C % N
E=E-1
RETURN D

���������ᷢ�֣�Ҫ֪��E ��ʱ������ 2��������Ҫ�������м�һ������Ĳ�����ֻ����֤E �Ķ����Ƹ�λ��0 ����1 �Ϳ����ˣ��������һ����������֤�����ԣ��������һ����࣬��E=Sum[i=0 to n](E*2**i)��0<=E<=1����

D=1
FOR i=n TO 0
D=D*D % N
IF E[i]=1 D=D*C % N
RETURN D

������ģ�������ת������һϵ�е�ģ�����㡣

 

ģ������

���ڳ�ģ���� A*B%N�����A��B����1024λ�Ĵ������ȼ���A*B����% N���ͻ����2048λ���м�������������ö�̬�ڴ���似���ͱ��뽫���������е�����ռ�����һ������������ɴ������˷ѣ���Ϊ�ھ����������²����õ��Ƕ����һ���ռ䣬�����ö�̬�ڴ���似����ʹ�����洢ʧȥ�����Զ�ʹ��������е�ѭ
��������÷ǳ�����������ģ���������Ҫԭ�����Ҫ����ֱ�Ӽ���A*B��

��A=Sum[i=0 to k](A[i]*r**i)��r=0x10000000��0<=A[i]<r����

������һ��ѭ��������

C=0;
FOR i=n to 0
C=C*r
C=C+A[i]*B 

RETURN C

������һ����λ�˷�ת������һϵ�е�λ�˷��ͼӷ������ڣ�

a*b %n = (a%n * b%n) %n
a+b %n = (a%n + b%n) %n

���ԣ�������C=A*B %N��������ȫ��������C�Ĺ�������ɣ�

C=0;
FOR i=n to 0
C=C*r %N
C=C+A[i]*B %N RETURN C

��������������м�����A*B ��C*r����������1056(1024+32)λ���ռ���ۻ�С�ö࣬����ʱ�����ȴ�Ӵ��ˣ���Ϊ��ģ�Ĺ�����һ�α���˶�Ρ����ڹ����ĳ�ģ�����������ʱ�任�ռ�Ľ��׻���ֵ�õģ����Ƕ��ڷ���ѭ���ĳ�ģ���㣬���ִ��۾��޷����ܣ�������Ѱ��·��

 

�ɸ�����ģ��Ҫ��������⣺

��ע���ɸ�����ģ��ʵ�����ǽ��������һ�����⣬����ʹ�ó���������λ�����������ģ������Ľ����ʱ��ע����ģ��������Ǵ�������Ļ����ϵġ���

�����磺������� R=10  һ����(������ʾ)23 =2*10^1+3*10^0  ������23 mod 5������ 23 *10**-K mod 5��ֵ ����ʹ�ó˷������ǿ��Բ�ȡ����İ취 ���ǽ� 23+5*q ��ʱ��Ӱ��ģ����Ľ�� ��23+5*q ��10�ı���ʱ �Ϳ�������λ��������10��һ��k��ֵȡ23����������λ�����ڽ���R�Ļ����ϣ�һֱ��λ�����ʣ��һ������ģ5 С��2*5�������ڼ�ȥһ��5 �������Ľ���ˣ������������23 * 10^-k mod 5 �Ľ������������23 mod 5�Ľ��ֻ��Ҫ�����23*10^-k mod 5 = Z����  ��ȥ�� z*10^k mod 5 ���� ����Ϊ 23 % 5 = (23 * 10**K * 10 ** (-K))%5 = ((23 * 10**(-K)%5 *10 ** (-K)%5)%5��

������Ӿٵò��ã���Ϊ23+5q ��ԶҲ�Ӳ���10�ı�������������RSA�㷨�������ԣ����ɸ�����ģ��ʱ�ǿ��Լӳ����Ʊ�������ʵ����λ�ģ��������ֻ����ᾫ��Ҳ���Բ�����

?�ɸ�����Լ�����ʽ�� Mon =��S+qM��/R = ��S+qM��*1/R  ��SΪ ����S��ʾ����Լ����M��ʾģ����R = 2^n������������£���n��ʾָ��0�ĸ���������S+qMʵ�ʱ�ʾģM��S���ڵ�ʣ�����������
?����ʣ��������㣺
 M(S)*M(1) = M(S)*M(R)*M(R^(-1))
����Mon mod M = S*R^(-1) mod M     ����������S*R^(-1) mod M��ֵ������Mon mod M�����㣬�ҵ�qʹ��S+qM��R�ı������������������������ˣ�
?Montgomery�˷���Z = X*Y*R^(-1) mod M
?(R��MҪ���أ���R��2^nʱ��M����������)��
 �ɸ�����ģ��������Ӳ��ʵ�֣���������ʵ��RSA���㣬�Լӿ������ٶȡ�
�ɸ�����ģ��(���б������Ǻ�������⣬���Բο������ע)


����RSA �ĺ����㷨��ģ�����㣬ģ���������൱��ģ�������ѭ����Ҫ���RSA �㷨��Ч�ʣ���Ҫ�����������ģ�������Ч�ʡ����ѷ��֣�ģ�˹����и��Ӷ���ߵĻ�������ģ���㣬��Ϊһ�γ���ʵ���ϰ����˶�μӷ��������ͳ˷���������㷨���ܹ��������ٳ�������������������㷨��Ч�ʻ�����ߡ�

��A=Sum[i=0 to k](A[i]*2**i)��0<=A[i]<=1����
C= A*B = Sum[i=0 to k](A*B*2**i)   ����ѭ������Ϊ��

C=0
FOR i FROM k TO 0
C=C*2
C=C+A[i]*B       
RETURN C

���� C'= A*B *2**(-k)��

C'= Sum[i=0 to k](A[i]*B*2**(i-k))

��ѭ��������

C'=0
FOR i FROM 0 TO k
C'=C'+A[i]*B
C'=C'/2
RETURN C'

ͨ����һ�㷨��A*B*2**(-k)�ǲ���ȷ�ģ���Ϊ��ѭ����ÿ�γ���2�������������������ˣ����ǿ���ͨ����һ�㷨��A*B*2**(-k) %N�ľ�ȷֵ���������ڶ�C'��
2֮ǰ����C'����C'[0]*N��������RSA��N�����������Ļ����������������Ե�C'������ʱ��C'[0]=1��C'+C'[0]*N ����ż��������C'Ϊż��ʱC'[0]=0��C'+C'[0]*N����ż��������C'/2 �Ͳ���������������������ΪC'+N %N = C' %N�������ڼ�������м����ɴ�N��������Ӱ��������ȷ�ԡ����Խ��㷨�������£�

{A*B*2**(-k)%N �ĺ������ҵ�һ����A*B%N ͬ���һ����H*2**K���ټ���H%N}

 

C'=0
FOR i FROM 0 TO k
C'=C'+A*B
C'=C'+C'[0]*N
C'=C'/2
IF C'>=N C'=C'-N
RETURN C'

 

������RSA��A��B����С��N����0<=A,C'[0]<=1�����ԣ�

C' = (C'+A*B+C'[0]*N)/2
C' < (C'+2N)/2
2C' < C'+2N
C' < 2N

��ȻC'����С��2N��������C' %N �Ϳ��Ժܼ򵥵��ڽ���ѭ������һ�μ�������ɣ�������A*B*2**(-k) %N�Ĺ����в��÷�����ģ���ﵽ�����Ǳ�����������Ŀ
�ġ���Ȼ����һ�㷨��õ���A*B*2**(-k) %N�����������������Ҫ��A*B %N����������A*B*2**(-k)����ͬ���������A**E %N��

��R=2**k %N��R'=2**(-k) %N��E=Sum[i=0 to n](E[i]*2**i)��

A'=A*R %N                //��һ������ô��ģ�
X=A'
FOR i FROM n TO 0
X=X*X*R' %N
IF E[i]=1 X=X*A'*R' %N
X=X*1*R' %N
RETURN X

�����

X = A*R %N��

��ʼѭ��ʱ��

X = X*X*R' %N
= A*R*A*R*R' %N 
= A**2*R %N

����ѭ��֮��

X = A**E*R %N

���

X = X*1*R' %N
= A**E*R*R' %N
= A**E %N

��ˣ���������ʵ���˲���������ģ���㷨��������������ɸ������㷨����X*Y*R' %N �򱻳�Ϊ���ɸ�����ģ�ˡ����������۵����ɸ�����ģ����򵥣�����
�����Ķ�������ʽ���ɸ������㷨�ĺ���˼�����ڽ���A*B %Nת��Ϊ����Ҫ����ȡģ��A*B*R' %N������λ���ɣ���ΪR��2^K,��֮R���������ص��������������ö������㷨��1024λ��A*B*R' %N����Ҫѭ��1024��֮�࣬��ô��Ȼϣ���ҵ�����Ч�ļ���A*B*R' %N���㷨��

{A*B%N

 =(A*B*2**(-K)  * 2**K )%N

 =(A*B*2**(-K)%N * 2**K%N)%N

 =((H*2**K * 2**(-K))%N * 2**K%N)%N

 =(H%N * 2**K%N)%N     //�����A*B*2**(-K) ͨ����λת������ H%N ��H��һ��С��2N��������H-N���ɡ�

}

���ǽ�A��ʾΪ�����r���ƣ�

A = Sum[i=0 to k](A*r**i) 0<=A<=r

������Ҫ�õ����ɸ������˻�Ϊ��

C'= A*B*R' %N R'=r**(-k)

�������㷨ֻ�ܵõ�C'�Ľ���ֵ

C'=0
FOR i FROM 0 TO k
C'=C'+A*B
C'=C'/r
IF C'>=N C'=C'-N
RETURN C'

��Ϊ��ѭ����ÿ��C'=C'/r ʱ�������������������������������ܹ��ҵ�һ��ϵ�� q��ʹ��(C' + A*B + q*N) %r =0�������㷨�޸�Ϊ��

C'=0
FOR i FROM 0 TO k
C'=C'+A*B+q*N
C'=C'/r
IF C'>=N C'=C'-N
RETURN C'

��C'�����շ���ֵ����A*B*R' %N�ľ�ȷֵ�����Թؼ�������q�����ڣ�

(C' + A*B + q*N) %r =0
==> (C' %r + A*B %r + q*N %r) %r =0
==> (C'[0] + A*B[0] + q*N[0]) %r =0

����N[0]*N[0]' %r =1��q=(C'[0]+A*B[0])*(r-N[0]') %r����

(C'[0] + A*B[0] + q*N[0]) %r
= (C'[0]+A*B[0] - (C'[0]+A*B[0])*N[0]'*N[0]) %r) %r
= 0

�������ǿ��Եó�rΪ�κ�ֵ���ɸ������㷨��

m=r-N[0]'
C'=0
FOR i FROM 0 TO k
q=(C'[0]+A*B[0])*m %r
C'=(C'+A*B+q*N)/r
IF C'>=N C'=C'-N
RETURN C'

����� r=0x100000000���� %r �� /r ���㶼���÷ǳ����ף���1024λ�������У�ѭ������k ������32��������������������м����C'=(C'+A*B+q*N)
< 2*r*N < 1057λ���㷨Ч�ʾ��൱���ˡ�Ψһ�Ķ��⸺������Ҫ���� N[0]'��ʹN[0]*N[0]' %r =1������һ����ǰ���Ѿ���ŷ������㷨������ˣ�������ģ������ת���ɷ���ģ������ʱ��N�ǹ̶�ֵ������N[0]'ֻ��Ҫ����һ�Σ�����������

 

�������Է���

����ѧ�����÷���С�����о����˶����������Է�����Ŀǰ�����㷨���������ղ����㷨����������£�

��1����������M��ʹ��N=(2**r)*M+1
��2��ѡ�������A<N
��3����������i<r����A**((2**i)*M) % N = N-1����Nͨ�������A�Ĳ���
��4�����ߣ���A**M % N = 1����Nͨ�������A�Ĳ���
��5����Aȡ��ͬ��ֵ��N����5�β��ԣ���ȫ��ͨ�����ж�NΪ����

��N ͨ��һ�β��ԣ���N ���������ĸ���Ϊ 25%����N ͨ��t �β��ԣ���N ���������ĸ���Ϊ1/4**t����ʵ��ȡt Ϊ5 ʱ��N ���������ĸ���Ϊ 1/128��N Ϊ�����ĸ����Ѿ�����99.99%��

��ʵ��Ӧ���У���������300��500��С������N ���в��ԣ�������������ղ���ͨ���ĸ��ʣ��Ӷ������������ٶȡ����������������ʱ��ѡȡ������������r=0�����ʡȥ���裨3�� �Ĳ��ԣ���һ����߲����ٶȡ�

����������RSA ѡȡ��Կ�ĵ�һ����������������������ӽ���ʱ�����������ȫһ�£�����ģ�����㡣��ģ���������������������ŷ����·�����ǡǡ
����ѡȡ��Կ�ڶ������õ����㡣�ɼ�����RSA �㷨����һ������ĺ�г��
*/
