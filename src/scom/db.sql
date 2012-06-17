
--------------------------------------------------------
DROP TABLE IF EXISTS public.scom_index CASCADE;
CREATE TABLE public.scom_index
(
    id bigserial NOT NULL PRIMARY KEY,
    password varchar NOT NULL,
    stage int4 NOT NULL DEFAULT 0,
    ctime TIMESTAMP WITHOUT TIME ZONE,
    atime TIMESTAMP WITHOUT TIME ZONE,
    dtime TIMESTAMP WITHOUT TIME ZONE
);
