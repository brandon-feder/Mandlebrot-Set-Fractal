class Mandlebrot {
    public:
        static int calc(double cr, double ci, int maxDepth) {
           return calc_sub(0, 0, cr, ci, maxDepth, 0);
            // return ci * 50;
        }

    private:
        static int calc_sub(double zr, double zi, double cr, double ci, int maxDepth, int depth) {
            // std::cout << depth << "\n";
            if(depth >= maxDepth) {
                return -1;
            } else if(calcDistSquare(zr, zi) > 4) {
                return depth;
            } else {
                return calc_sub(zr*zr-zi*zi+cr, 2*zi*zr+ci, cr, ci, maxDepth, depth+1);
            }
        } 

        static double calcDistSquare(double x1, double x2) {
            return x1*x1 + x2*x2;
        }
};