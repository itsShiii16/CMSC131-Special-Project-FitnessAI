#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class DecisionTree {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        if (x[3] <= 0.5) {
                            if (x[0] <= 29.5) {
                                if (x[2] <= 24.949999809265137) {
                                    return 2;
                                }

                                else {
                                    return 1;
                                }
                            }

                            else {
                                if (x[0] <= 49.5) {
                                    return 1;
                                }

                                else {
                                    return 0;
                                }
                            }
                        }

                        else {
                            if (x[0] <= 29.5) {
                                if (x[2] <= 24.850000381469727) {
                                    return 5;
                                }

                                else {
                                    return 4;
                                }
                            }

                            else {
                                if (x[0] <= 49.5) {
                                    return 4;
                                }

                                else {
                                    return 3;
                                }
                            }
                        }
                    }

                protected:
                };
            }
        }
    }