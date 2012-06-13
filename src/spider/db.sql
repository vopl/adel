--
-- PostgreSQL database dump
--

-- Dumped from database version 9.1.2
-- Dumped by pg_dump version 9.1.2
-- Started on 2012-06-12 21:10:07

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;


SET search_path = public, pg_catalog;

SET default_with_oids = false;

--
-- TOC entry 161 (class 1259 OID 1782574)
-- Dependencies: 5
-- Name: page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS page CASCADE;
CREATE TABLE page (
    id bigint NOT NULL,
    site_id bigint,
    uri character varying,
    body_length bigint,
    headers character varying,
    status character varying,
    get_time integer
);


--
-- TOC entry 162 (class 1259 OID 1782580)
-- Dependencies: 161 5
-- Name: page_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE page_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1912 (class 0 OID 0)
-- Dependencies: 162
-- Name: page_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE page_id_seq OWNED BY page.id;


--
-- TOC entry 1913 (class 0 OID 0)
-- Dependencies: 162
-- Name: page_id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('page_id_seq', 1, true);


--
-- TOC entry 163 (class 1259 OID 1782582)
-- Dependencies: 5
-- Name: reference; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS reference CASCADE;
CREATE TABLE reference (
    id bigint NOT NULL,
    from_id bigint,
    to_id bigint NOT NULL
);


--
-- TOC entry 164 (class 1259 OID 1782585)
-- Dependencies: 163 5
-- Name: reference_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE reference_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1914 (class 0 OID 0)
-- Dependencies: 164
-- Name: reference_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE reference_id_seq OWNED BY reference.id;


--
-- TOC entry 1915 (class 0 OID 0)
-- Dependencies: 164
-- Name: reference_id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('reference_id_seq', 1, false);


--
-- TOC entry 165 (class 1259 OID 1782587)
-- Dependencies: 1884 1885 1886 1887 1888 1889 1890 1891 1892 5
-- Name: site; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS site CASCADE;
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


--
-- TOC entry 166 (class 1259 OID 1782602)
-- Dependencies: 5 165
-- Name: site_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE site_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1916 (class 0 OID 0)
-- Dependencies: 166
-- Name: site_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE site_id_seq OWNED BY site.id;


--
-- TOC entry 1917 (class 0 OID 0)
-- Dependencies: 166
-- Name: site_id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('site_id_seq', 1, true);


--
-- TOC entry 167 (class 1259 OID 1782604)
-- Dependencies: 5
-- Name: word2; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word2 CASCADE;
CREATE TABLE word2 (
    id bigint NOT NULL,
    word1 integer,
    word2 integer
);


--
-- TOC entry 168 (class 1259 OID 1782607)
-- Dependencies: 167 5
-- Name: word2_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE word2_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1918 (class 0 OID 0)
-- Dependencies: 168
-- Name: word2_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE word2_id_seq OWNED BY word2.id;


--
-- TOC entry 1919 (class 0 OID 0)
-- Dependencies: 168
-- Name: word2_id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('word2_id_seq', 1, false);


--
-- TOC entry 169 (class 1259 OID 1782609)
-- Dependencies: 5
-- Name: word2_to_page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word2_to_page CASCADE;
CREATE TABLE word2_to_page (
    word2_id bigint,
    page_id bigint
);


--
-- TOC entry 170 (class 1259 OID 1782612)
-- Dependencies: 5
-- Name: word3; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word3 CASCADE;
CREATE TABLE word3 (
    id bigint NOT NULL,
    word1 integer,
    word2 integer,
    word3 integer
);


--
-- TOC entry 171 (class 1259 OID 1782615)
-- Dependencies: 170 5
-- Name: word3_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE word3_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 1920 (class 0 OID 0)
-- Dependencies: 171
-- Name: word3_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE word3_id_seq OWNED BY word3.id;


--
-- TOC entry 1921 (class 0 OID 0)
-- Dependencies: 171
-- Name: word3_id_seq; Type: SEQUENCE SET; Schema: public; Owner: -
--

SELECT pg_catalog.setval('word3_id_seq', 1, false);


--
-- TOC entry 172 (class 1259 OID 1782617)
-- Dependencies: 5
-- Name: word3_to_page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word3_to_page CASCADE;
CREATE TABLE word3_to_page (
    word3_id bigint,
    page_id bigint
);


--
-- TOC entry 1882 (class 2604 OID 1782620)
-- Dependencies: 162 161
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE page ALTER COLUMN id SET DEFAULT nextval('page_id_seq'::regclass);


--
-- TOC entry 1883 (class 2604 OID 1782621)
-- Dependencies: 164 163
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE reference ALTER COLUMN id SET DEFAULT nextval('reference_id_seq'::regclass);


--
-- TOC entry 1893 (class 2604 OID 1782622)
-- Dependencies: 166 165
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE site ALTER COLUMN id SET DEFAULT nextval('site_id_seq'::regclass);


--
-- TOC entry 1894 (class 2604 OID 1782623)
-- Dependencies: 168 167
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE word2 ALTER COLUMN id SET DEFAULT nextval('word2_id_seq'::regclass);


--
-- TOC entry 1895 (class 2604 OID 1782624)
-- Dependencies: 171 170
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE word3 ALTER COLUMN id SET DEFAULT nextval('word3_id_seq'::regclass);


--
-- TOC entry 1900 (class 0 OID 1782574)
-- Dependencies: 161
-- Data for Name: page; Type: TABLE DATA; Schema: public; Owner: -
--

INSERT INTO page (id, site_id, uri, body_length, headers, status) VALUES (1, 1, 'http://127.0.0.1:8080/index.html', NULL, NULL, NULL);


--
-- TOC entry 1901 (class 0 OID 1782582)
-- Dependencies: 163
-- Data for Name: reference; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1902 (class 0 OID 1782587)
-- Dependencies: 165
-- Data for Name: site; Type: TABLE DATA; Schema: public; Owner: -
--

INSERT INTO site (id, name, address, priority, amount_ref_incoming, amount_ref_outgoing, amount_page_all, amount_page_new, amount_page_update, amount_page_dead, time_per_page, time_access) VALUES (1, '127.0.0.1:8080', '127.0.0.1', 1, 0, 0, 1, 1, 0, 0, '00:00:10', '2012-06-12 21:06:36.558');


--
-- TOC entry 1903 (class 0 OID 1782604)
-- Dependencies: 167
-- Data for Name: word2; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1904 (class 0 OID 1782609)
-- Dependencies: 169
-- Data for Name: word2_to_page; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1905 (class 0 OID 1782612)
-- Dependencies: 170
-- Data for Name: word3; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1906 (class 0 OID 1782617)
-- Dependencies: 172
-- Data for Name: word3_to_page; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1897 (class 2606 OID 1782628)
-- Dependencies: 161 161
-- Name: page_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY page
    ADD CONSTRAINT page_pkey PRIMARY KEY (id);


--
-- TOC entry 1899 (class 2606 OID 1782626)
-- Dependencies: 165 165
-- Name: site_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY site
    ADD CONSTRAINT site_pkey PRIMARY KEY (id);


-- Completed on 2012-06-12 21:10:08

--
-- PostgreSQL database dump complete
--

CREATE UNIQUE INDEX word2_words_idx ON word2 (word1,word2);
CREATE UNIQUE INDEX word3_words_idx ON word3 (word1,word2,word3);







CREATE OR REPLACE function link_page_word2(page_id bigint, w1 int4, w2 int4) returns bigint AS
$$
DECLARE
  mID bigint;
  cID CURSOR IS SELECT id FROM word2 WHERE word1=w1 AND word2=w2;
BEGIN
  OPEN cid;
  FETCH cID INTO mID;
  CLOSE cid;
  IF NOT FOUND
  THEN
    BEGIN
        INSERT INTO word2 (word1,word2) VALUES (w1,w2) RETURNING id INTO mID;
    EXCEPTION WHEN unique_violation THEN
      OPEN cid; 
      FETCH cID INTO mID;
      CLOSE cid;
    END;
  END IF;

  INSERT INTO word2_to_page (word2_id, page_id) VALUES (mID,page_id);
  RETURN mID;
END;
$$language plpgsql;





CREATE OR REPLACE function link_page_word3(page_id bigint, w1 int4, w2 int4, w3 int4) returns bigint AS
$$
DECLARE
  mID bigint;
  cID CURSOR IS SELECT id FROM word3 WHERE word1=w1 AND word2=w2 AND word3=w3;
BEGIN
  OPEN cid;
  FETCH cID INTO mID;
  CLOSE cid;
  IF NOT FOUND
  THEN
    BEGIN
        INSERT INTO word3 (word1,word2,word3) VALUES (w1,w2,w3) RETURNING id INTO mID;
    EXCEPTION WHEN unique_violation THEN
      OPEN cid; 
      FETCH cID INTO mID;
      CLOSE cid;
    END;
  END IF;

  INSERT INTO word3_to_page (word3_id, page_id) VALUES (mID,page_id);
  RETURN mID;
END;
$$language plpgsql;



