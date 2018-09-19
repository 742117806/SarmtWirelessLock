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

file://大数相加
file://调用形式：N.Add(A)，返回值：N+A
file://若两大数符号相同，其值相加，否则改变参数符号再调用大数相减函数
/ ****************************************************************** /
例如：
     A  B  C
+       D  E
--------------
= S  F  G  H

其中，若C+E<=0xffffffff，则H=C+E，carry(进位标志)=0
     若C+E>0xffffffff，则H=C+E-0x100000000，carry=1

     若B+D+carry<=0xfffffff，则G=B+D，carry=0      
     若B+D+carry>0xfffffff，则G=B+D+carry-0x10000000，carry=1

     若carry=0，则F=A，S=0
     若carry=1，A<0xfffffff，则F=A+1，S=0
     若carry=1，A=0xfffffff，则F=0，S=1
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

file://大数相减
file://调用形式：N.Sub(A)，返回值：N-A
file://若两大数符号相同，其值相减，否则改变参数符号再调用大数相加函数
/ ****************************************************************** /
例如：
     A  B  C
-       D  E
--------------
=    F  G  H

其中，若C>=E，则H=C-E，carry(借位标志)=0
     若C<E，则H=C-E+0x100000000，carry=1

     若B-carry>=D，则G=B-carry-D，carry=0      
     若B-carry<D，则G=B-carry-D+0x10000000，carry=1

     若carry=0，则F=A
     若carry=1，A>1，则F=A-1
     若carry=1，A=1，则F=0
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

file://大数相乘
file://调用形式：N.Mul(A)，返回值：N*A
/ ****************************************************************** /
例如：
        A  B  C
*          D  E
----------------
=    S  F  G  H
+ T  I  J  K
----------------
= U  V  L  M  N

其中，SFGH=ABC*E，TIJK=ABC*D

而对于：
     A  B  C
*          E
-------------
= S  F  G  H   

其中，若C*E<=0xffffffff，则H=C*E，carry(进位标志)=0
     若C*E>0xffffffff，则H=(C*E)&0xffffffff
       carry=(C*E)/0xffffffff
     若B*E+carry<=0xffffffff，则G=B*E+carry，carry=0
     若B*E+carry>0xffffffff，则G=(B*E+carry)&0xffffffff
       carry=(B*E+carry)/0xffffffff
     若A*E+carry<=0xffffffff，则F=A*E+carry，carry=0
     若A*E+carry>0xffffffff，则F=(A*E+carry)&0xffffffff
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

file://大数相除
file://调用形式：N.Div(A)，返回值：N/A
file://除法的关键在于“试商”，然后就变成了乘法和减法
file://这里将被除数与除数的试商转化成了被除数最高位与除数最高位的试商
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

file://大数求模
file://调用形式：N.Mod(A)，返回值：N%A
file://求模与求商原理相同
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


file://暂时只给出了十进制字符串的转化
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

file://暂时只给出了十进制字符串的转化
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

file://欧几里德算法求：Y=X.Euc(A)，使满足：YX mod A=1
file://相当于对不定方程ax-by=1求最小整数解
file://实际上就是初中学过的辗转相除法
/ ******************************************************************** /
例如：11x-49y=1，求x

           11 x  -  49 y  =   1      a)
49%11=5 ->  11 x  -   5 y  =   1      b)
11%5 =1 ->     x  -   5 y  =   1      c)

令y=1  代入c)式  得x=6
令x=6  代入b)式  得y=13
令y=13 代入a)式  得x=58  
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

file://蒙哥马利算法求：Y=X.Mon(A,B)，使满足：X^A mod B=Y
file://俺估计就是高中学过的反复平方法
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


最后需要说明的是因为在VC里面存在一个__int64类型可以
用来计算进位与借位值，所以将大数当作0x100000000进制
进行运算是可能的，而在其他编译系统中如果不存在64位
整形，则可以采用0x40000000进制，由于在0x40000000
进制中，对任何两个“数字”进行四则运算，结果都在
0x3fffffff*03fffffff之间，小于0xffffffff，都可以用
一个32位无符号整数来表示。事实上《楚汉棋缘》采用的
freelip大数库正是运用了0x40000000进制来表示大数的，
所以其反汇编后大数的值在内存中表现出来有些“奇怪”。




////////////////////////////////////////////////////////////////////////
// BigInt.cpp: implementation of the BigInt class.
//
//////////////////////////////////////////////////////////////////////
/ *****************************************************************
大数运算库源文件：BigInt.cpp
说明：适用于MFC，1024位RSA运算
***************************************************************** /
#include "stdafx.h"
#include "BigInt.h"
//小素数表
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
//构造大数对象并初始化为零
CBigInt::CBigInt()
{
    m_nLength=1;
    for(int i=0;i<BI_MAXLEN;i++)m_ulValue[i]=0;
}
//解构大数对象
CBigInt::~CBigInt()
{
}
/ ****************************************************************************************
大数比较
调用方式：N.Cmp(A)
返回值：若N<A返回-1；若N=A返回0；若N>A返回1
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
大数赋值
调用方式：N.Mov(A)
返回值：无，N被赋值为A
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
大数相加
调用形式：N.Add(A)
返回值：N+A
**************************************************************************************** /
CBigInt CBigInt::Add(CBigInt& A)
{
    CBigInt X;
    X.Mov(*this);//X被赋值为N
    unsigned carry=0;
    unsigned __int64 sum=0;
    if(X.m_nLength<A.m_nLength)X.m_nLength=A.m_nLength;
    for(unsigned i=0;i<X.m_nLength;i++)
    {
        sum=A.m_ulValue[i];
  sum=sum+X.m_ulValue[i]+carry;
        X.m_ulValue[i]=(unsigned long)sum;
        carry=(unsigned)(sum>>32);//判断是否有进位，有进位则carry=1,否则为0；
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
大数相减
调用形式：N.Sub(A)
返回值：N-A
**************************************************************************************** /
CBigInt CBigInt::Sub(CBigInt& A)
{
    CBigInt X;
    X.Mov(*this);
    if(X.Cmp(A)<=0){X.Mov(0);return X;}//如果N比A小的话就返回0；
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
    X.m_ulValue[i]--;//借位运算；
    if(X.m_ulValue[i]==0)X.m_nLength--;
    return X;
}
/ ****************************************************************************************
大数相乘
调用形式：N.Mul(A)
返回值：N*A
**************************************************************************************** /
CBigInt CBigInt::Mul(CBigInt& A)
{
    if(A.m_nLength==1)return Mul(A.m_ulValue[0]);//如果长度为一调用重载函数
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
        carry=(unsigned long)(mul>>32);//进位
    }
    if(carry){X.m_nLength++;X.m_ulValue[X.m_nLength-1]=carry;}
    return X;
}
/ ****************************************************************************************
大数相除
调用形式：N.Div(A)
返回值：N/A
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
大数求模
调用形式：N.Mod(A)
返回值：N%A
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
求最大公约数；调用方式N.Gcd(A,B);
Gcd（A，B）；返回值为A，B的最大公约数d->N；
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
从字符串按10进制或16进制格式输入到大数
调用格式：N.Get(str,sys)
返回值：N被赋值为相应大数
sys暂时只能为10或16
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
将大数按10进制或16进制格式输出为字符串
调用格式：N.Put(str,sys)
返回值：无，参数str被赋值为N的sys进制字符串
sys暂时只能为10或16
**************************************************************************************** /
void CBigInt::Put(CString& str, unsigned int system)
{
    if((m_nLength==1)&&(m_ulValue[0]==0)){str="0";return;}
 str="";
    CString t="0123456789ABCDEF";
    int a;
    char ch;
    CBigInt X;
    X.Mov(*this);//X被赋值为STR；
    while(X.m_ulValue[X.m_nLength-1]>0)
    {
        a=X.Mod(system);
        ch=t[a];
        str.Insert(0,ch);
        X.Mov(X.Div(system));//X=X/system;
    }
}
/ ****************************************************************************************
求不定方程ax-by=1的最小整数解
调用方式：N.Euc(A)
返回值：X,满足：NX mod A=1
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
求乘方的模
调用方式：N.RsaTrans(A,B)
返回值：X=N^A MOD B
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
拉宾米勒算法测试素数
调用方式：N.Rab()
返回值：若N为素数，返回1，否则返回0
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
产生随机素数
调用方法：N.GetPrime(bits)
返回值：N被赋值为一个bits位（0x100000000进制长度）的素数
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
实现快速模乘的Montgomery算法，它和反复平方法结合可以实现大数的快速
模乘。调用形式：N.(A,B,R,N)
***************************************************************** /
/ ****************************************************************************************
求不定方程ax-by=1的最小整数解
调用方式：N.Euc(A)
返回值：X,满足：NX mod A=1
**************************************************************************************** /
void CBigInt::Mon(CBigInt A,CBigInt B,unsigned long R,CBigInt N)
{
    CBigInt R_1,n;
 n.Mov(N);
 unsigned long temp,N_1;
 //生成一个与大数N互素的正整数；
   do{
       R=rand();//产生随机数并且检验是否与大数N互素；
        temp=Gcd(N,R);
   }while(temp!=1);
   //求出R*(R的模N逆)=1Mod(N);的解；即求R的模N逆；
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
//  printf("字符串1的长度: length1=%d\n",len_num1);  
    i=0;  
    while(str_num2[i]!='\0')  
    {  
        len_num2++;  
        i++;  
    }     
//  printf("字符串2的长度: length2=%d\n\n",len_num2);  
  
    tempbuf1[0]='0';  
    tempbuf2[0]='0';  
  
//=======================================================================  
    if(len_num2>=len_num1)                                   //补成相同长度  
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
/ *  printf("-------------------------------------------------------\n转换之后:\n"); 
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
  
    while(tempbuf1[i]!='\0')  // 字符串长度  
    {  
        len++;  
        i++;  
    }  
  
    int temp[200]={0};        // 该数组用来存储各次方的系数 10为底  
  
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
    int num=i; // 科学计数法次数  
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
void  ChenFang(char *tempbuf1,   int num, char *result)    // 大数乘方  
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
void  Mod(char *tempbuf1,   char *tempbuf2, char *result , char *div_result )    // 大数求余  
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
            Sub (tempbuf1,tempbuf2,AfterSub);   //减去之后在判断是否得到结果----即：tempbuf1 < tempbuf2  
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
    else  // if (Compare(tempbuf1,tempbuf2)==-1) // 后者大的话，mod 就是 tempbuf1  
    {  
        strcpy (result,tempbuf1);  
        ThrowAway_0 (result);  
    }  
}  
  
//========================================================  
  
//extern "C" __declspec(dllexport)   
void Add(char *tempbuf1,   char *tempbuf2, char *result) // 大数相加 result = tempbuf1 + tempbuf2  
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
  
void  ThrowAway_0 (char *tempbuf )  // 去除结果前面的 连续的无意义的 "0"  
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
void Sub(char *tempbuf1, char *tempbuf2, char *result)     // 大数相减 result = tempbuf1 - tempbuf2  
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
  
    while(tempbuf1[i]!='\0')    // tempbuf1 和 tempbuf2 的长度相等  
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
2.头文件

[cpp] view plain copy
#include <stdio.h>   
#include <string.h>  
#include <stdlib.h>  
  
//extern "C" __declspec(dllexport)   
void Add (char *tempbuf1,   char *tempbuf2, char *result);  // 大数相加 result = tempbuf1 + tempbuf2  
  
//extern "C" __declspec(dllexport)   
void Sub(char *tempbuf1, char *tempbuf2, char *result);     // 大数相减 result = tempbuf1 - tempbuf2 (默认前者大)  
  
int Compare(char *tempbuf1,char *tempbuf2);  
  
//extern "C" __declspec(dllexport)    
void Chen (char *tempbuf1,   char *tempbuf2, char *result); // 大数相乘 result = tempbuf1 * tempbuf2  
  
void Trans (char *str_num1,   char *str_num2, char *tempbuf1, char *tempbuf2); // 大数转化为等长，且最前面添加 “0”  
  
  
//extern "C" __declspec(dllexport)                                              // 大数除法 div_result = tempbuf1 / tempbuf2  
void  Mod(char *tempbuf1,   char *tempbuf2, char *result , char *div_result );    // 大数求余 result = tempbuf1 % tempbuf2    
  
  
//extern "C" __declspec(dllexport)   
void  ChenFang (char *tempbuf1,   int num, char *result);    // 大数乘方   result = tempbuf1 ^ num  
  
void  ThrowAway_0 (char *tempbuf );  // 去除结果前面的 连续的无意义的 "0"  


原理介绍

RSA 原理：

选取两个不同的大素数p、q，并计算N=p*q，选取小素数d，并计算e，使d*e % (p-1)(q-1)=1，
对于任意A<N：
若B=A**d % N
则A=B**e % N

可见d、e形成了非对称秘钥关系，加密者用公钥d加密，解密者可用私钥e解密，第三者即使拦截了密文B、公钥d和N，在不知道p、q的前提下，无法推算出e，从而无法获得明文A。当N取非常大的值时，将其因式分解成p、q是非常困难的，例如当N为1024 bit时，据分析，需动用价值数千万美金的大型计算机系统并耗费一年的时间。

RSA 密钥的选取和加解密过程都非常简洁，在算法上主要要实现四个问题：

1、如何处理大数运算
2、如何求解同余方程 XY % M = 1
3、如何快速进行模幂运算
4、如何获取大素数

实际上，在实现RSA 算法的过程中大家会发现后三个问题不是各自独立的，它们互有关联，环环相套，相信届时你会意识到：RSA算法是一种“优美”的算法！

 

大数存储：

RSA 依赖大数运算，目前主流RSA 算法都建立在1024位的大数运算之上。而大多数的编译器只能支持到64位的整数运算，即我们在运算中所使用的整数必须小于等于64位，即：0xffffffffffffffff，也就是18446744073709551615，这远远达不到RSA 的需要，于是需要专门建立大数运算库来解决这一问题。

最简单的办法是将大数当作数组进行处理，数组的各元素也就是大数每一位上的数字，通常采用最容易理解的十进制数字0—9。然后对“数字数组”编写加减乘除函数。但是这样做效率很低，因为二进制为1024位的大数在十进制下也有三百多位，对于任何一种运算，都需要在两个有数百个元素的数组空间上多次重循环，还要许多额外的空间存放计算的进退位标志及中间结果。另外，对于某些特殊的运算而言，采用二进制会使计算过程大大简化，而这种大数表示方法转化成二进制显然非常麻烦，所以在某些实例中则干脆采用了二进制数组的方法来记录大数，当然这样效率就更低了。

一个有效的改进方法是将大数表示为一个n 进制数组，对于目前的32位系统而言n 可以取值为2 的32次方，即 0x100000000，假如将一个二进制为1024位的大数
转化成0x10000000进制，就变成了32位，而每一位的取值范围不再是二进制的0—1或十进制的0—9，而是0-0xffffffff，我们正好可以用一个32位的DWORD （如：无符号长整数，unsigned long） 类型来表示该值。所以1024位的大数就变成一个含有32个元素的 DWORD数组，而针对 DWORD数组进行各种运算所需的循环规模至多32次而已。而且0x100000000 进制与二进制，对于计算机来说，几乎是一回事，转换非常容易。

例如大数18446744073709551615，等于 0xffffffff ffffffff，就相当于十进制的99：有两位，每位都是0xffffffff。而18446744073709551616等于0x00000001 00000000 00000000，就相当于十进制的100：有三位，第一位是1 ，其它两位都是0 ，如此等等。在实际应用中，“数字数组”的排列顺序采用低位在前高位在后的方式，这样，大数A 就可以方便地用数学表达式来表示其值：
A=Sum[i=0 to n](A[i]*r**i)，r=0x100000000，0<=A<r
其中Sum 表示求和，A[i]表示用以记录A 的数组的第i 个元素，**表示乘方。

任何整数运算最终都能分解成数字与数字之间的运算，在0x100000000 进制下其“数字”最大达到0xffffffff，其数字与数字之间的运算，结果也必然超出了目前32位系统的字长。在VC++中，存在一个__int64 类型可以处理64位的整数，所以不用担心这一问题，而在其它编译系统中如果不存在64位整形，就需要采用更小的
进制方式来存储大数，例如16位的WORD类型可以用来表示0x10000 进制。但效率更高的办法还是采用32位的 DWORD类型，只不过将0x100000000 进制改成0x40000000进制，这样两个数字进行四则运算的最大结果为 0x3fffffff * 0x3fffffff，小于0xffffffffffffff，可以用一个双精度浮点类型（double，52位有效数字）来储存这一中间结果，只是不能简单地用高位低位来将中间结果拆分成两个“数字”。

 

大数加减乘除：

设有大数A、B和C，其中A>=B：
A=Sum[i=0 to p](A[i]*r**i)
B=Sum[i=0 to q](B[i]*r**i)
C=Sum[i=0 to n](C[i]*r**i)
r=0x100000000(32位)，0<=A[i],B[i],C[i]<r，p>=q(A[i],B[i],C[i]都是32位的数)
则当C=A+B、C=A-B、C=A*B时，我们都可以通过计算出C来获得C：

1.加法
C=A+B，显然C[i]不总是等于A[i]+B[i]，因为A[i]+B[i]可能>0xffffffff，而C[i]必须<=0xffffffff，这时就需要进位，当然在计算C[i-1]时也可能产生了进位，所以计算C[i]时还要加上上次的进位值。如果用一个64位变量result来记录和（64位是为乘法准备的，实际加减法只要33位即可），另一个32位变量carry来记录进位(为什么要32位？为乘法准备的，实际加减法进位只有1)，则有：

carry=0;
for(i=0;i<=p;i++) ｛      //i从0到p 因为A>B
　　result=A[i]+B[i]+carry;
　　C[i]=result%0x100000000 ;   //从这里看result应该大于64位，至少65位
　　carry=result/0x100000000;
｝
if(carry=0) n=p;
else n=p+1;

2.减法
C=A-B，同理C[i]不总是等于A[i]-B[i]，因为A[i]-B[i]可能<0，而C[i]必须>=0，这时就需要借位，同样在计算C[i-1]时也可能产生了借位，所以计算C时还要减去上次的借位值：

carry=0
for(i=0;i<=p;i++) {     //i从0到p 因为A>B
　　if((A[i]-B[i]-carry)>=0){
　　　　C[i]=A[i]-B[i]-carry;
　　　　carry=0;
　　}
　　else{
　　　　C[i]=0x100000000+A[i]-B[i]-carry;
　　　　carry=1;
　　}
}
n=p;
while (C[n]==0) n=n-1;   //将前边的0去掉

3.乘法
C=A*B，首先我们需要观察日常做乘法所用的“竖式计算”过程：

　　　　　　  　　　　　　　A3 A2 A1 A0
　　　　　　    　　　　　　　* B2 B1 B0
------------------------------------------
　　　　　 　　= A3B0 A2B0 A1B0 A0B0
　　　　 + A3B1 A2B1 A1B1 A0B1
    + A3B2 A2B2 A1B2 A0B2
------------------------------------------
=        C5     C4     C3    C2    C1     C0

可以归纳出：C[i]=Sum[j=0 to q](A[i-j]*B[j]) (注意是C[i])，其中i-j必须>=0且<=p。
当然这一结论没有考虑进位，虽然计算A[i-j]*B[j]和Sum的时候都可能发生进位，显然这两种原因产生的进位可以累加成一个进位值。最终可用如下算法完成乘法：
C = Sum[i= 0 to n](C[i]*r**i) =  Sum[i= 0 to n] ( Sum[j=0 to q](A[i-j]*B[j])  *r**i).(这里的n=p+q-1，但当第n位的运算有进位时n应加1)
C也可以表示成 C= Sum[i= 0 to q](A*B[i] *r**i)

n=p+q-1
carry=0
for(i=0;i<=n;i++){
　　result=carry;
　　for(j=0;j<=q;j++){
　　　　if (0<=i-j<=p ){
　　　　　　result=result+A[i-j]*B[j];
　　　　　　C[i]=result%0x100000000;
　　　　　　carry=result/0x100000000;
　　　　}

　　}

}
if(carry!=0) {
　　n=n+1;
　　C[n]=carry
} 

4.除法
对于C=A/B，由于无法将B 对A“试商”，我们只能转换成B[q]对A[p]的试商来得到一个近似值，所以无法直接通过计算C来获得C，只能一步步逼近C。由于：
B*(A[p]/B[q]-1)*0x100000000**(p-q)<C

令：X=0，重复A=A-X*B，X=X+(A[p]/B[q]-1)*0x100000000**(p-q)，(为什么？)  直到A<B
则：X=A/B，且此时的A=A%B

注意对于任意大数A*0x100000000**k，都等价于将A 的数组中的各元素左移k 位，
不必计算；同样，A/0x100000000**k则等价于右移。

 

欧几里得方程：

在RSA 算法中，往往要在已知A、N的情况下，求 B，使得 (A*B)%N=1。即相当于求解B、M都是未知数的二元一次不定方程 A*B-N*M=1 的最小整数解。
而针对不定方程ax-by=c 的最小整数解，古今中外都进行过详尽的研究，西方有著名的欧几里德算法，即辗转相除法，中国有秦九韶的“大衍求一术”。事实上二元一次不定方程有整数解的充要条件是c为a、b的最大公约数。即当c=1时，a、b必须互质。而在RSA算法里由于公钥d为素数，素数与任何正整数互质，所以可以通
过欧几里德方程来求解私钥e。

欧几里德算法是一种递归算法，比较容易理解：

例如：11x-49y=1，求x
（a） 11 x - 49 y = 1 49%11=5 ->
（b） 11 x - 5 y = 1 11%5 =1 ->
（c） x - 5 y = 1
令y=0 代入（c）得x=1
令x=1 代入（b）得y=2
令y=2 代入（a）得x=9

同理可使用递归算法求得任意 ax-by=1（a、b互质）的解。实际上通过分析归纳将递归算法转换成非递归算法就变成了大衍求一术：

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

 

模幂运算

模幂运算是RSA 的核心算法，最直接地决定了RSA 算法的性能。针对快速模幂运算这一课题，西方现代数学家提出了大量的解决方案，通常都是先将幂模运算转化为乘模运算。

例如求D=C**15 % N，由于：a*b % n = (a % n)*(b % n) % n，所以：

C1 =C*C % N =C**2 % N
C2 =C1*C % N =C**3 % N
C3 =C2*C2 % N =C**6 % N
C4 =C3*C % N =C**7 % N
C5 =C4*C4 % N =C**14 % N
C6 =C5*C % N =C**15 % N

即：对于E=15的幂模运算可分解为6 个乘模运算，归纳分析以上方法可以发现对于任意E，都可采用以下算法计算D=C**E % N：

D=1
WHILE E>=0
IF E%2=0
C=C*C % N
E=E/2
ELSE
D=D*C % N
E=E-1
RETURN D

继续分析会发现，要知道E 何时能整除 2，并不需要反复进行减一或除二的操作，只需验证E 的二进制各位是0 还是1 就可以了，从左至右或从右至左验证都可以，从左至右会更简洁，设E=Sum[i=0 to n](E*2**i)，0<=E<=1，则：

D=1
FOR i=n TO 0
D=D*D % N
IF E[i]=1 D=D*C % N
RETURN D

这样，模幂运算就转化成了一系列的模乘运算。

 

模乘运算

对于乘模运算 A*B%N，如果A、B都是1024位的大数，先计算A*B，再% N，就会产生2048位的中间结果，如果不采用动态内存分配技术就必须将大数定义中的数组空间增加一倍，这样会造成大量的浪费，因为在绝大多数情况下不会用到那额外的一倍空间，而采用动态内存分配技术会使大数存储失去连续性而使运算过程中的循
环操作变得非常繁琐。所以模乘运算的首要原则就是要避免直接计算A*B。

设A=Sum[i=0 to k](A[i]*r**i)，r=0x10000000，0<=A[i]<r，则：

可以用一个循环来处理：

C=0;
FOR i=n to 0
C=C*r
C=C+A[i]*B 

RETURN C

这样将一个多位乘法转换成了一系列单位乘法和加法，由于：

a*b %n = (a%n * b%n) %n
a+b %n = (a%n + b%n) %n

所以，对于求C=A*B %N，我们完全可以在求C的过程中完成：

C=0;
FOR i=n to 0
C=C*r %N
C=C+A[i]*B %N RETURN C

这样产生的最大中间结果是A*B 或C*r，都不超过1056(1024+32)位，空间代价会小得多，但是时间代价却加大了，因为求模的过程由一次变成了多次。对于孤立的乘模运算而言这种时间换空间的交易还是值得的，但是对于反复循环的乘模运算，这种代价就无法承受，必须另寻出路。

 

蒙哥马利模乘要解决的问题：

｛注：蒙哥马利模乘实际上是解决了这样一个问题，即不使用除法（用移位操作）而求得模乘运算的结果。时刻注意是模运算而且是大数运算的基础上的。｝

｛例如：假设进制 R=10  一个数(大数表示)23 =2*10^1+3*10^0  。欲求23 mod 5，先求 23 *10**-K mod 5的值 ，不使用乘法，我们可以采取下面的办法 就是将 23+5*q 这时不影响模运算的结果 当23+5*q 是10的倍数时 就可以用移位操作除以10，一般k的值取23（大数）的位数（在进制R的基础上）一直移位，最后剩下一个大于模5 小于2*5的数，在减去一个5 就是最后的结果了，这样求出的是23 * 10^-k mod 5 的结果。我们想求23 mod 5的结果只需要先求出23*10^-k mod 5 = Z即可  在去求 z*10^k mod 5 即可 ，因为 23 % 5 = (23 * 10**K * 10 ** (-K))%5 = ((23 * 10**(-K)%5 *10 ** (-K)%5)%5。

这个例子举得不好，因为23+5q 永远也加不出10的倍数，但是由于RSA算法的特殊性，在蒙哥马利模乘时是可以加出进制倍数，而实现移位的，这个例子只是领会精神，也可以不看｝

?蒙哥马利约减表达式： Mon =（S+qM）/R = （S+qM）*1/R  （S为 其中S表示被归约数，M表示模数，R = 2^n（二进制情况下），n表示指定0的个数。）。S+qM实际表示模M的S所在的剩余类的所有数
?看做剩余类的运算：
 M(S)*M(1) = M(S)*M(R)*M(R^(-1))
所以Mon mod M = S*R^(-1) mod M     （所以想求S*R^(-1) mod M的值可以用Mon mod M来计算，找到q使得S+qM是R的倍数，这样被除数就是整数了）
?Montgomery乘法：Z = X*Y*R^(-1) mod M
?(R与M要互素，当R是2^n时，M是奇数即可)｝
 蒙哥马利模乘器可用硬件实现，在配合软件实现RSA运算，以加快运算速度。
蒙哥马利模乘(下列表述不是很容易理解，可以参考上面的注)


由于RSA 的核心算法是模幂运算，模幂运算又相当于模乘运算的循环，要提高RSA 算法的效率，首要问题在于提高模乘运算的效率。不难发现，模乘过程中复杂度最高的环节是求模运算，因为一次除法实际上包含了多次加法、减法和乘法，如果在算法中能够尽量减少除法甚至避免除法，则算法的效率会大大提高。

设A=Sum[i=0 to k](A[i]*2**i)，0<=A[i]<=1，则：
C= A*B = Sum[i=0 to k](A*B*2**i)   可用循环处理为：

C=0
FOR i FROM k TO 0
C=C*2
C=C+A[i]*B       
RETURN C

若令 C'= A*B *2**(-k)则

C'= Sum[i=0 to k](A[i]*B*2**(i-k))

用循环处理即：

C'=0
FOR i FROM 0 TO k
C'=C'+A[i]*B
C'=C'/2
RETURN C'

通过这一算法求A*B*2**(-k)是不精确的，因为在循环中每次除以2都可能有余数被舍弃了，但是可以通过这一算法求A*B*2**(-k) %N的精确值，方法是在对C'除
2之前，让C'加上C'[0]*N。由于在RSA中N是两个素数的积，总是奇数，所以当C'是奇数时，C'[0]=1，C'+C'[0]*N 就是偶数，而当C'为偶数时C'[0]=0，C'+C'[0]*N还是偶数，这样C'/2 就不会有余数被舍弃。又因为C'+N %N = C' %N，所以在计算过程中加若干次N，并不会影响结果的正确性。可以将算法整理如下：

{A*B*2**(-k)%N 的含义是找到一个与A*B%N 同余的一个数H*2**K，再计算H%N}

 

C'=0
FOR i FROM 0 TO k
C'=C'+A*B
C'=C'+C'[0]*N
C'=C'/2
IF C'>=N C'=C'-N
RETURN C'

 

由于在RSA中A、B总是小于N，又0<=A,C'[0]<=1，所以：

C' = (C'+A*B+C'[0]*N)/2
C' < (C'+2N)/2
2C' < C'+2N
C' < 2N

既然C'总是小于2N，所以求C' %N 就可以很简单地在结束循环后用一次减法来完成，即在求A*B*2**(-k) %N的过程中不用反复求模，达到了我们避免做除法的目
的。当然，这一算法求得的是A*B*2**(-k) %N，而不是我们最初需要的A*B %N。但是利用A*B*2**(-k)我们同样可以求得A**E %N。

设R=2**k %N，R'=2**(-k) %N，E=Sum[i=0 to n](E[i]*2**i)：

A'=A*R %N                //这一步是怎么求的？
X=A'
FOR i FROM n TO 0
X=X*X*R' %N
IF E[i]=1 X=X*A'*R' %N
X=X*1*R' %N
RETURN X

最初：

X = A*R %N，

开始循环时：

X = X*X*R' %N
= A*R*A*R*R' %N 
= A**2*R %N

反复循环之后：

X = A**E*R %N

最后：

X = X*1*R' %N
= A**E*R*R' %N
= A**E %N

如此，我们最终实现了不含除法的模幂算法，这就是著名的蒙哥马利算法，而X*Y*R' %N 则被称为“蒙哥马利模乘”。以上讨论的是蒙哥马利模乘最简单，最容
易理解的二进制形式。蒙哥马利算法的核心思想在于将求A*B %N转化为不需要反复取模的A*B*R' %N，（移位即可，因为R是2^K,总之R是与进制相关的数），但是利用二进制算法求1024位的A*B*R' %N，需要循环1024次之多，我么必然希望找到更有效的计算A*B*R' %N的算法。

{A*B%N

 =(A*B*2**(-K)  * 2**K )%N

 =(A*B*2**(-K)%N * 2**K%N)%N

 =((H*2**K * 2**(-K))%N * 2**K%N)%N

 =(H%N * 2**K%N)%N     //这里把A*B*2**(-K) 通过移位转化成了 H%N 而H是一个小于2N的数，做H-N即可。

}

考虑将A表示为任意的r进制：

A = Sum[i=0 to k](A*r**i) 0<=A<=r

我们需要得到的蒙哥马利乘积为：

C'= A*B*R' %N R'=r**(-k)

则以下算法只能得到C'的近似值

C'=0
FOR i FROM 0 TO k
C'=C'+A*B
C'=C'/r
IF C'>=N C'=C'-N
RETURN C'

因为在循环中每次C'=C'/r 时，都可能有余数被舍弃。假如我们能够找到一个系数 q，使得(C' + A*B + q*N) %r =0，并将算法修改为：

C'=0
FOR i FROM 0 TO k
C'=C'+A*B+q*N
C'=C'/r
IF C'>=N C'=C'-N
RETURN C'

则C'的最终返回值就是A*B*R' %N的精确值，所以关键在于求q。由于：

(C' + A*B + q*N) %r =0
==> (C' %r + A*B %r + q*N %r) %r =0
==> (C'[0] + A*B[0] + q*N[0]) %r =0

若令N[0]*N[0]' %r =1，q=(C'[0]+A*B[0])*(r-N[0]') %r，则：

(C'[0] + A*B[0] + q*N[0]) %r
= (C'[0]+A*B[0] - (C'[0]+A*B[0])*N[0]'*N[0]) %r) %r
= 0

于是我们可以得出r为任何值的蒙哥马利算法：

m=r-N[0]'
C'=0
FOR i FROM 0 TO k
q=(C'[0]+A*B[0])*m %r
C'=(C'+A*B+q*N)/r
IF C'>=N C'=C'-N
RETURN C'

如果令 r=0x100000000，则 %r 和 /r 运算都会变得非常容易，在1024位的运算中，循环次数k 不大于32，整个运算过程中最大的中间变量C'=(C'+A*B+q*N)
< 2*r*N < 1057位，算法效率就相当高了。唯一的额外负担是需要计算 N[0]'，使N[0]*N[0]' %r =1，而这一问题前面已经用欧几里德算法解决过了，而且在模幂运算转化成反复模乘运算时，N是固定值，所以N[0]'只需要计算一次，负担并不大。

 

素数测试方法

数论学家利用费马小定理研究出了多种素数测试方法，目前最快的算法是拉宾米勒测试算法，其过程如下：

（1）计算奇数M，使得N=(2**r)*M+1
（2）选择随机数A<N
（3）对于任意i<r，若A**((2**i)*M) % N = N-1，则N通过随机数A的测试
（4）或者，若A**M % N = 1，则N通过随机数A的测试
（5）让A取不同的值对N进行5次测试，若全部通过则判定N为素数

若N 通过一次测试，则N 不是素数的概率为 25%，若N 通过t 次测试，则N 不是素数的概率为1/4**t。事实上取t 为5 时，N 不是素数的概率为 1/128，N 为素数的概率已经大于99.99%。

在实际应用中，可首先用300—500个小素数对N 进行测试，以提高拉宾米勒测试通过的概率，从而提高整体测试速度。而在生成随机素数时，选取的随机数最好让r=0，则可省去步骤（3） 的测试，进一步提高测试速度。

素数测试是RSA 选取秘钥的第一步，奇妙的是其核心运算与加解密时所需的运算完全一致：都是模幂运算。而模幂运算过程中中所需求解的欧几里德方程又恰恰
正是选取密钥第二步所用的运算。可见整个RSA 算法具有一种整体的和谐。
*/
