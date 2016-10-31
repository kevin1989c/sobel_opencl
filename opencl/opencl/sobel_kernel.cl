kernel void sobel(int xsize, int ysize,int thresh, global int *pic, global int *result)
{

int magnitude,sum1,sum2;
int i = get_global_id(0);
int j = get_global_id(1);



sum1 =  pic[ xsize * (i-1) + j+1 ] -     pic[ xsize*(i-1) + j-1 ] 
        + 2 * pic[ xsize * (i)   + j+1 ] - 2 * pic[ xsize*(i)   + j-1 ]
        +     pic[ xsize * (i+1) + j+1 ] -     pic[ xsize*(i+1) + j-1 ];
      
sum2 = pic[ xsize * (i-1) + j-1 ] + 2 * pic[ xsize * (i-1) + j ]  + pic[ xsize * (i-1) + j+1 ]
            - pic[xsize * (i+1) + j-1 ] - 2 * pic[ xsize * (i+1) + j ] - pic[ xsize * (i+1) + j+1 ];

int offset = i*xsize + j;
      
magnitude =  sum1*sum1 + sum2*sum2;

      if (magnitude > thresh)
        result[offset] = 255;
      else 
        result[offset] = 0;
}
