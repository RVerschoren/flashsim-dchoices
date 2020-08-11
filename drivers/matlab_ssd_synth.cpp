#include "MatlabDataArray.hpp"

int main() {
    using namespace matlab::data;
    ArrayFactory factory;

    // Create a 3-by-2 TypedArray
    TypedArray<double>  A = factory.createArray( {3,2},
        {1.1, 2.2, 3.3, 4.4, 5.5, 6.6 });

    // Define scalar multiplier
    double multiplier(10.2);

    // Multiple each element in A
    for (auto& elem : A) {
        elem *= multiplier;
    }

    return 0;
}
