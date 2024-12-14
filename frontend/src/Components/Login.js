import React, {useEffect, useState} from "react";
import {Container, Form, Button} from 'react-bootstrap';
import logo from '../Assets/logo.PNG';
import axios from "axios";
import { useContextApi } from "../ContexApi";
import { useNavigate } from "react-router-dom";
import { toast } from "react-toastify";
export default function Login(){
    const [email, setEmail] = useState("");
    const [password, setPassword] = useState("");
    const navigate = useNavigate();

    const {token, setToken, login, setLogin, setUserData} = useContextApi();

    const handleLogin = async(e) => {
        var resp = await axios.post("http://localhost:2028/api/login/", {email, password}).catch((err) =>  toast.error("Invalid Credentials!"));
        if (resp.status == 200) {
            setToken(resp.data.token);
            localStorage.setItem("token", resp.data.token);
            setLogin(true);
            toast.success("Login Successful, You will be redirected shortly!");
            setTimeout(()=>(navigate("/")),1000);
        }
        
        
    }

    useEffect(() => {
        if (login) {
            navigate("/");
        }
    }, [login]);
    
    return(
        <div style={{display:'flex'}}>
            <Container className="sidebar" style={{minHeight: "100vh", maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
                <p className="h2 text-center mt-5" style={{color:'white'}}>Navigation</p>
                <Container className="sidebar-links">
                    <p className="nav-btn text-center active">Login</p>
                </Container>
            </Container>
            <Container className="main" style={{padding:'0', margin:'0', minHeight:'100vh'}}>
                <Container className="header" style={{width:'100%', border:'1px solid black', padding: '0', margin:'0', minHeight:'15%'}}>
                    <Container style={{padding: '20px', textAlign:'center', display:'flex', alignItems:'center', justifyContent:'center'}}>
                        <img src={logo} style={{width:'50px', height:'50px', marginRight:'10px'}} alt="logo" />
                        <p className="h1">StudentSync</p>
                    </Container>
                </Container>
                <Container className="mainContent" style={{backgroundColor:'#D4D4D4', minHeight:'85%',display:'flex',flexDirection:'column'}}>
                    <Container style={{marginTop:'50px'}}>
                        <h2 className="text-center">Login</h2>
                        <p className="text-center">Please login to continue</p>
                    </Container>
                    <Form style={{width:'50%', margin:'0 auto'}}>
                        <Form.Group className="mb-3">
                            <Form.Label style={{marginBottom:'-20px'}}>Username/Email</Form.Label>
                            <Form.Control style={{marginBottom:'10px'}} value={email} onChange={(e) => setEmail(e.target.value)} placeholder="Enter email" />
                            <Form.Label style={{marginBottom:'-20px'}}>Password</Form.Label>
                            <Form.Control style={{marginBottom:'10px'}} value={password} onChange={(e) => setPassword(e.target.value)} type="password" placeholder="Enter password" />
                        </Form.Group>
                        <Container style={{display:'flex', justifyContent:'center'}}>
                            <Button variant="primary" style={{backgroundColor:"#2463AB"}} onClick={handleLogin}>
                                Login
                            </Button>
                        </Container>
                    </Form>
                </Container>
            </Container>
        </div>
    )
}