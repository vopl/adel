--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: page; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE page (
    id bigint NOT NULL,
    site_id bigint,
    uri character varying,
    body_length bigint,
    headers character varying,
    status character varying
);


ALTER TABLE public.page OWNER TO spider;

--
-- Name: page_id_seq; Type: SEQUENCE; Schema: public; Owner: spider
--

CREATE SEQUENCE page_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.page_id_seq OWNER TO spider;

--
-- Name: page_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: spider
--

ALTER SEQUENCE page_id_seq OWNED BY page.id;


--
-- Name: page_id_seq; Type: SEQUENCE SET; Schema: public; Owner: spider
--

SELECT pg_catalog.setval('page_id_seq', 1, false);


--
-- Name: reference; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE reference (
    id bigint NOT NULL,
    from_id bigint,
    to_id bigint NOT NULL
);


ALTER TABLE public.reference OWNER TO spider;

--
-- Name: reference_id_seq; Type: SEQUENCE; Schema: public; Owner: spider
--

CREATE SEQUENCE reference_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.reference_id_seq OWNER TO spider;

--
-- Name: reference_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: spider
--

ALTER SEQUENCE reference_id_seq OWNED BY reference.id;


--
-- Name: reference_id_seq; Type: SEQUENCE SET; Schema: public; Owner: spider
--

SELECT pg_catalog.setval('reference_id_seq', 1, false);


--
-- Name: site; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE site (
    id bigint NOT NULL,
    name character varying NOT NULL,
    address character varying NOT NULL,
    priority double precision DEFAULT 1.0 NOT NULL,
    amount_ref_incoming integer DEFAULT 0 NOT NULL,
    amount_ref_outgoing integer DEFAULT 0 NOT NULL,
    amount_page_all integer DEFAULT 0 NOT NULL,
    amount_page_new integer DEFAULT 0 NOT NULL,
    amount_page_update integer DEFAULT 0 NOT NULL,
    amount_page_dead integer DEFAULT 0 NOT NULL,
    time_per_page interval DEFAULT '00:00:10'::interval NOT NULL,
    time_access timestamp without time zone DEFAULT now() NOT NULL
);


ALTER TABLE public.site OWNER TO spider;

--
-- Name: site_id_seq; Type: SEQUENCE; Schema: public; Owner: spider
--

CREATE SEQUENCE site_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.site_id_seq OWNER TO spider;

--
-- Name: site_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: spider
--

ALTER SEQUENCE site_id_seq OWNED BY site.id;


--
-- Name: site_id_seq; Type: SEQUENCE SET; Schema: public; Owner: spider
--

SELECT pg_catalog.setval('site_id_seq', 1, false);


--
-- Name: word2; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE word2 (
    id bigint NOT NULL,
    word1 integer,
    word2 integer
);


ALTER TABLE public.word2 OWNER TO spider;

--
-- Name: word2_id_seq; Type: SEQUENCE; Schema: public; Owner: spider
--

CREATE SEQUENCE word2_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.word2_id_seq OWNER TO spider;

--
-- Name: word2_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: spider
--

ALTER SEQUENCE word2_id_seq OWNED BY word2.id;


--
-- Name: word2_id_seq; Type: SEQUENCE SET; Schema: public; Owner: spider
--

SELECT pg_catalog.setval('word2_id_seq', 1, false);


--
-- Name: word2_to_page; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE word2_to_page (
    word2_id bigint,
    page_id bigint
);


ALTER TABLE public.word2_to_page OWNER TO spider;

--
-- Name: word3; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE word3 (
    id bigint NOT NULL,
    word1 integer,
    word2 integer,
    word3 integer
);


ALTER TABLE public.word3 OWNER TO spider;

--
-- Name: word3_id_seq; Type: SEQUENCE; Schema: public; Owner: spider
--

CREATE SEQUENCE word3_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.word3_id_seq OWNER TO spider;

--
-- Name: word3_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: spider
--

ALTER SEQUENCE word3_id_seq OWNED BY word3.id;


--
-- Name: word3_id_seq; Type: SEQUENCE SET; Schema: public; Owner: spider
--

SELECT pg_catalog.setval('word3_id_seq', 1, false);


--
-- Name: word3_to_page; Type: TABLE; Schema: public; Owner: spider; Tablespace: 
--

CREATE TABLE word3_to_page (
    word3_id bigint,
    page_id bigint
);


ALTER TABLE public.word3_to_page OWNER TO spider;

--
-- Name: id; Type: DEFAULT; Schema: public; Owner: spider
--

ALTER TABLE ONLY page ALTER COLUMN id SET DEFAULT nextval('page_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: spider
--

ALTER TABLE ONLY reference ALTER COLUMN id SET DEFAULT nextval('reference_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: spider
--

ALTER TABLE ONLY site ALTER COLUMN id SET DEFAULT nextval('site_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: spider
--

ALTER TABLE ONLY word2 ALTER COLUMN id SET DEFAULT nextval('word2_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: spider
--

ALTER TABLE ONLY word3 ALTER COLUMN id SET DEFAULT nextval('word3_id_seq'::regclass);


--
-- Data for Name: page; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY page (id, site_id, uri, body_length, headers, status) FROM stdin;
\.


--
-- Data for Name: reference; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY reference (id, from_id, to_id) FROM stdin;
\.


--
-- Data for Name: site; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY site (id, name, address, priority, amount_ref_incoming, amount_ref_outgoing, amount_page_all, amount_page_new, amount_page_update, amount_page_dead, time_per_page, time_access) FROM stdin;
\.


--
-- Data for Name: word2; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY word2 (id, word1, word2) FROM stdin;
\.


--
-- Data for Name: word2_to_page; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY word2_to_page (word2_id, page_id) FROM stdin;
\.


--
-- Data for Name: word3; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY word3 (id, word1, word2, word3) FROM stdin;
\.


--
-- Data for Name: word3_to_page; Type: TABLE DATA; Schema: public; Owner: spider
--

COPY word3_to_page (word3_id, page_id) FROM stdin;
\.


--
-- Name: site_pkey; Type: CONSTRAINT; Schema: public; Owner: spider; Tablespace: 
--

ALTER TABLE ONLY site
    ADD CONSTRAINT site_pkey PRIMARY KEY (id);


--
-- Name: public; Type: ACL; Schema: -; Owner: local
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM local;
GRANT ALL ON SCHEMA public TO local;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

