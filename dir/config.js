const FORWARD = 1;
const BACKWARD = 0;
const REDIRECT = 2;
const config = 
{
    l: 2,
    d: FORWARD,
    1: {
        d: FORWARD,
        l: 2,
        1: {
            d: FORWARD,
            l: 2,
            1: {
                d: REDIRECT
            },
            2: {
                d: BACKWARD
            }
        },
        2: {
            l: 3,
            d: FORWARD,
            1: {
                d: FORWARD,
            },
            2: {
                d: BACKWARD
            },
            3: {
                d: REDIRECT
            }
        }
    },
    2: {
        d: FORWARD
    }
};
console.log(JSON.stringify(config))