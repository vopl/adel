
--------------------------------------------------------
DROP TABLE IF EXISTS public.scom_index CASCADE;
CREATE TABLE public.scom_index
(
    id bigserial NOT NULL PRIMARY KEY,
    login varchar NOT NULL,
    password varchar NOT NULL,
    ctime TIMESTAMP WITHOUT TIME ZONE,
    atime TIMESTAMP WITHOUT TIME ZONE
);
