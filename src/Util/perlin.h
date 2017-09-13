class Perlin{
public:
	static double noise(double x, double y, double z);

private:
		static double fade(double t);
		static double lerp(double t, double a, double b);
		static double grad(int hash, double x, double y, double z);
		static int p[512];
};
