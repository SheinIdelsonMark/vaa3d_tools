#include "mean_shift_fun.h"

const float windowsize=15;

mean_shift_fun::mean_shift_fun()
{
    data1Dc_float=0;
    sz_image[0]=sz_image[1]=sz_image[2]=sz_image[3]=0;
    page_size=0;
}

mean_shift_fun::~mean_shift_fun()
{
    if(data1Dc_float!=0)
        memory_free_float1D(data1Dc_float);
}

vector<V3DLONG> mean_shift_fun::calc_mean_shift_center(V3DLONG ind)
{
    qDebug()<<"In mean_shift_fun now";
    qDebug()<<"image size:"<<sz_image[0]<<":"<<sz_image[1]<<":"<<sz_image[2];
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG x,y,z,pos;//,center_x,center_y,center_z;
    vector<V3DLONG> coord;

    coord=pos2xyz(ind, y_offset, z_offset);
    x=coord[0];y=coord[1];z=coord[2];
//    center_x=x; center_y=y; center_z=z;

    qDebug()<<"ind:"<<ind<<"x:"<<x<<" y:"<<y<<" z:"<<z;
    float total_x,total_y,total_z,v_color,sum_v;
    float center_dis=1;
    vector<V3DLONG> center(3,0);

    while (center_dis>=1)
    {
        total_x=total_y=total_z=sum_v=0;

        for(V3DLONG dx=MAX(x-15,0); dx<=MIN(sz_image[0]-1,x+15); dx++){
            for(V3DLONG dy=MAX(y-15,0); dy<=MIN(sz_image[1]-1,y+15); dy++){
                for(V3DLONG dz=MAX(z-15,0); dz<=MIN(sz_image[2]-1,z+15); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowsize) continue;
                    v_color=data1Dc_float[pos];

                    for (int j=1;j<sz_image[3];j++)
                    v_color=MAX(v_color,data1Dc_float[pos+page_size*j]);

                    total_x=v_color*dx+total_x;
                    total_y=v_color*dy+total_y;
                    total_z=v_color*dz+total_z;
                    sum_v=sum_v+v_color;
                 }
             }
         }
            qDebug()<<"v_color:"<<v_color<<":"<<"total xyz:"<<total_x<<":"<<total_y<<":"<<total_z<<":"<<sum_v;
            center[0]=total_x/sum_v;
            center[1]=total_y/sum_v;
            center[2]=total_z/sum_v;
        float tmp_1=(center[0]-x)*(center[0]-x)+(center[1]-y)*(center[1]-y)
                    +(center[2]-z)*(center[2]-z);
        center_dis=sqrt(tmp_1);
        qDebug()<<"center distance:"<<center_dis<<":"<<center[0]<<":"<<center[1]<<":"<<center[2];
        x=center[0]; y=center[1]; z=center[2];
    }
    return center;
}

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size)
{
    unsigned char *ptr_result;
    ptr_result=(unsigned char *) calloc(i_size, sizeof(unsigned char));
    return(ptr_result);
}

void memory_free_uchar1D(unsigned char *ptr_input)
{
    free(ptr_input);
}

float * memory_allocate_float1D(const V3DLONG i_size)
{
    float *ptr_result;
    ptr_result=(float *) calloc(i_size, sizeof(float));
    return(ptr_result);
}

void memory_free_float1D(float *ptr_input)
{
    free(ptr_input);
}

vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
{
    vector<V3DLONG> poss_result;
    V3DLONG count_landmark=LandmarkList_input.count();
    for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
    {
        poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
    }
    return poss_result;
}

V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z)
{
    float x=0;
    float y=0;
    float z=0;
    Landmark_input.getCoord(x, y, z);
    x=MAX(x,0);
    y=MAX(y,0);
    z=MAX(z,0);
    return (xyz2pos(x-1, y-1, z-1, _offset_Y, _offset_Z));
}

vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    vector<V3DLONG> pos3_result (3, -1);
    pos3_result[2]=floor(_pos_input/(double)_offset_Z);
    pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
    pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
    return pos3_result;
}

V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    return _z*_offset_Z+_y*_offset_Y+_x;
}