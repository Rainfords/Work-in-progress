  int p[] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};
  
  // To remove the need for index wrapping, double the permutation table length
  int perm[512];

  struct Grad
  {
	  float x, y, z;
	  Grad() : x(0), y(0), z(0)
	  {
	  }
	  Grad(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	  {
	  }
	  float dot2(float _x, float _y)
	  {
		  return x*_x + y*_y;
	  }
  };

  Grad grad3[] = {Grad(1,1,0), Grad(-1,1,0), Grad(1,-1,0), Grad(-1,-1,0),
               Grad(1,0,1), Grad(-1,0,1), Grad(1,0,-1), Grad(-1,0,-1),
			   Grad(0,1,1), Grad(0,-1,1),Grad(0,1,-1), Grad(0,-1,-1)};
  Grad gradP[512];

class CNoise
{
public:
  void seed(int seed) 
  {
    if(seed < 256) 
      seed |= seed << 8;

    for(int i = 0; i < 256; i++) 
	{
      int v;
      if (i & 1) {
        v = p[i] ^ (seed & 255);
      } else {
        v = p[i] ^ ((seed>>8) & 255);
      }

      perm[i] = perm[i + 256] = v;
      gradP[i] = gradP[i + 256] = grad3[v % 12];
    }
  };


  // 2D simplex noise
  float simplex2(float xin, float yin) 
  {
    const float F2 = 0.5f*(sqrt(3.0f)-1);
	const float G2 = (3.0f-sqrt(3.0f))/6.0f;

    float n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    float s = (xin+yin)*F2; // Hairy factor for 2D
    int i = (int)floor(xin+s);
    int j = (int)floor(yin+s);
    float t = (i+j)*G2;
    float x0 = xin-i+t; // The x,y distances from the cell origin, unskewed.
    float y0 = yin-j+t;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) { // lower triangle, XY order: (0,0)->(1,0)->(1,1)
      i1=1; j1=0;
    } else {    // upper triangle, YX order: (0,0)->(0,1)->(1,1)
      i1=0; j1=1;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1 + 2 * G2; // Offsets for last corner in (x,y) unskewed coords
    float y2 = y0 - 1 + 2 * G2;

    // Work out the hashed gradient indices of the three simplex corners
    i &= 255;
    j &= 255;
    Grad gi0 = gradP[i+perm[j]];
    Grad gi1 = gradP[i+i1+perm[j+j1]];
    Grad gi2 = gradP[i+1+perm[j+1]];

    // Calculate the contribution from the three corners
    float t0 = 0.5f - x0*x0-y0*y0;
    if(t0<0) {
      n0 = 0;
    } else {
      t0 *= t0;
      n0 = t0 * t0 * gi0.dot2(x0, y0);  // (x,y) of grad3 used for 2D gradient
    }

    float t1 = 0.5f - x1*x1-y1*y1;
    if(t1<0) {
      n1 = 0;
    } else {
      t1 *= t1;
      n1 = t1 * t1 * gi1.dot2(x1, y1);
    }

    float t2 = 0.5f - x2*x2-y2*y2;
    if(t2<0) {
      n2 = 0;
    } else {
      t2 *= t2;
      n2 = t2 * t2 * gi2.dot2(x2, y2);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
  };

};

class CWorldGenerator
{
public:
	static void Generate(uint8_t* pWorld, int nWidth, int nHeight, uint8_t base, int nSeed)
	{
		RandSeed() = nSeed;
		CreatePalette();
		CNoise noise;
		noise.seed(nSeed);

		unsigned long i=0;
		for (int y=0; y<nHeight; y++)
			for (int x=0; x<nWidth; x++, i++)
				pWorld[i] = base + Mapper(Crusher(noise.simplex2(wrap(x / (float)nWidth), wrap(y / (float)nHeight))));
	}

	static float wrap(float x)
	{
      x = sin(x*3.1415f);
	  return x * x * 2.3f;
	}

	static uint32_t& RandSeed()
	{
		static uint32_t nSeed = 1;
		return nSeed;
	}

	static uint16_t NextRand()
	{
		uint32_t& r = RandSeed();
		r = 36969 * (r & 65535) + (r >> 16);
		return (uint16_t)r;
	}

	static float Crusher(float fValue)
	{
		if ( fValue < -0.5f )
		{
			fValue += ((NextRand() % 100) / 50.0f - 1.0f) * 0.05f;
			if ( fValue > -0.501f )
				fValue = -0.501f;

			return fValue;
		}

		fValue += ((NextRand() % 100) / 50.0f - 1.0f) * 0.05f;
		if ( fValue < -0.40f )
			fValue = -0.40f;

		return fValue;
	}

	static int Mapper(float fValue)
	{
		const float fWaterThresh = -0.5f;
		const float fMountainThresh = 0.6f;
		const float fMin = -0.8f;
		const float fMax = 1.0f;

		// fMin..fWaterThresh -> 0x00
		// fWaterThresh..fMountainThresh -> 0x10
		// fMountainThresh..fMax -> 0x20;

		if ( fValue < fMin )
			return 0;

		if ( fValue < fWaterThresh )
			return (int)((fValue-fMin)/(fWaterThresh-fMin)*0x10);

		if ( fValue < fMountainThresh )
			return (int)((fValue-fWaterThresh)/(fMountainThresh-fWaterThresh)*0x10)+0x10;

		if ( fValue < fMax )
			return (int)((fValue-fMountainThresh)/(fMax-fMountainThresh)*0x10)+0x20;

		return 0x2f;
	}

	static void CreatePalette()
	{
		COLORREF* pPalette = GetPalette();
		for (int i=0; i<0x32; i++)
			pPalette[i] = RGB(0xff, 0x00, 0xff);

		for (float f=-1.0f; f<=1.0f; f+= 0.01f)
		{
			int y = (int)(f*128+128);
			int r = y, g = y, b = y;

			if (f < -0.3f)
				b = 255;
			else if (f < 0.6 )
			{
				g = 128;
				b = 0;
			}

			int i = Mapper(f);
			if ( r < 0 ) r = 0; if ( r > 255 ) r = 255;
			if ( g < 0 ) g = 0; if ( g > 255 ) g = 255;
			if ( b < 0 ) b = 0; if ( b > 255 ) b = 255;

			pPalette[i] = RGB(b, g, r);
		}
	}

	static COLORREF* GetPalette()
	{
		static COLORREF arrPalette[0x32];
		arrPalette[0x30] = RGB(0x30, 0x60, 0xa0); // dirt
		arrPalette[0x31] = RGB(0x30, 0x30, 0x30); // track
		return arrPalette;
	}
};
