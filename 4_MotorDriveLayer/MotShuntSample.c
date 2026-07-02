#include "sysdef.h"
#include "lks32mc07x.h"

//Tdb+Ton_delay+Tsettle= 2.2us + (0.034+0.023) + 2us ≈ 4.3us,4.3*120=516
#define u_sample_minWidth_before0  (516u)
//Tsh+Tconv = 3*tsh+2*tconv=0.625us, 0.625*120=75
#define u_sample_minWidth_after0   (75u)

typedef struct
{
    uint16_t    minSampTime;
    uint16_t    cmprU;
    uint16_t    cmprV;
    uint16_t    cmprW;
    uint16_t    indexOrder[3];
    uint16_t    indexOrderLast[3];
    uint16_t    flagOnce;
} SHUNT_SAMPLE_T;

SHUNT_SAMPLE_T  shuntSample;

void sortThreePhasePWMDuty(uint16_t cmprU, uint16_t cmprV, uint16_t cmprW);
void rebuildThreePhaseCurr(int16_t *currU, int16_t *currV, int16_t *currW);

/********************************************************************************
* Function name: sortThreePhasePWMDuty
* @brief
* @param[in]:
* @param[out]:
* @retval:
* @author: homeson
* @date 2024-09-19
* @note 写完三相 EPWM 比较值之后立刻调用，传入的是已写入硬件的三相比较寄存器数值
*       这样可以保证在v>u>w的情况下，采样到的电流是v>u>w的顺序
*       这样可以保证在v>u>w的情况下，采样到的电流是v>u>w的顺序
********************************************************************************/
void sortThreePhasePWMDuty(uint16_t cmprU, uint16_t cmprV, uint16_t cmprW)
{
    if (0 == shuntSample.flagOnce)
    {
		//shuntSample.minSampTime = u_sample_minWidth_before0;
        shuntSample.indexOrder[0] = 0;
        shuntSample.indexOrder[1] = 1;
        shuntSample.indexOrder[2] = 2;
        shuntSample.flagOnce = 1;
    }

    shuntSample.indexOrderLast[0] = shuntSample.indexOrder[0];
    shuntSample.indexOrderLast[1] = shuntSample.indexOrder[1];
    shuntSample.indexOrderLast[2] = shuntSample.indexOrder[2];

    // 判断ccr值大小，把shuntSample.indexOrder[0]-shuntSample.indexOrder[2]中填入uvw的大小顺序 \
    其中，0代表u，1代表v，2代表w；如，w > u > v，则填入：indexOrder[0] = 2(w)，[1] = 0(u)，[2] = 1(v)；
	if (cmprU >= cmprV)
    {
        if (cmprU >= cmprW)
        {
            shuntSample.indexOrder[0] = 0; //max,u最大, 0代表u
            if (cmprV >= cmprW)
            {
                shuntSample.indexOrder[1] = 1; //mid, v中间, 1代表v
                shuntSample.indexOrder[2] = 2; //min，w最小, 2代表w
            }
            else
            {
                shuntSample.indexOrder[1] = 2;//mid, w中间, 2代表w
                shuntSample.indexOrder[2] = 1;//min, v最小, 1代表v
            }
        }
        else    //v<=u<=w
        {
            shuntSample.indexOrder[0] = 2;//2代表w
            shuntSample.indexOrder[1] = 0;
            shuntSample.indexOrder[2] = 1;
        }
    }
    else //v>u
    {
        if (cmprV >= cmprW) //v>w
        {
            shuntSample.indexOrder[0] = 1;
            if (cmprU >= cmprW) //v>u>w
            {
                shuntSample.indexOrder[1] = 0;
                shuntSample.indexOrder[2] = 2;
            }
            else //v>w>u
            {
                shuntSample.indexOrder[1] = 2;
                shuntSample.indexOrder[2] = 0;
            }
        }
        else //w>v>u
        {
            shuntSample.indexOrder[0] = 2;
            shuntSample.indexOrder[1] = 1;
            shuntSample.indexOrder[2] = 0;
        }
    }
}

/********************************************************************************
* Function name: rebuildThreePhaseCurr
* @brief
* @param[in]:
* @param[out]:
* @retval:
* @author:
* @date
* @note rebuildThreePhaseCurr()根据\shuntSample.indexOrderLast[0](上一拍里 cmpr 最小的那一相)，决定重写哪一相电流
        cmpr最小的那一相在此采样策略下被认为分流采样窗口不理想,\
        因此不用该相 ADC 作为主结果，而用另外两相由基尔霍夫推出该相电流。
		indexOrderLast[]从0到2存储的是三相的占空比，第一个成员：[0]存的是占空比最大的相，占空比越大越难采，所以
********************************************************************************/
void rebuildThreePhaseCurr(int16_t *currU, int16_t *currV, int16_t *currW)
{
#if 0
    if (shuntSample.indexOrderLast[2] == 0) //最小的元素是0（0代表U相）->cmprU min,means low bridge on min
    {
        *(currU) = -(*(currV)) - (*(currW));
    }
    else if (shuntSample.indexOrderLast[2] == 1)
    {
        *(currV) = -(*(currU)) - (*(currW));
    }
    else
    {
        *(currW) = -(*(currV)) - (*(currU));
    }
#else
    if (shuntSample.indexOrderLast[0] == 0) //cmprU min,means low bridge on min
    {
        *(currU) = -(*(currV)) - (*(currW));
    }
    else if (shuntSample.indexOrderLast[0] == 1)
    {
        *(currV) = -(*(currU)) - (*(currW));
    }
    else
    {
        *(currW) = -(*(currV)) - (*(currU));
    }
#endif
}
