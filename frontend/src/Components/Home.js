import React, {useState, useEffect} from "react";
import {Container, Form, Button} from 'react-bootstrap';
import logo from '../Assets/logo.PNG';
import axios from "axios";
import { useContextApi } from "../ContexApi";
import { useNavigate } from "react-router-dom";
export default function Home(){

    const {token, setToken, login, setLogin} = useContextApi();
    const navigate = useNavigate();
    
    useEffect(() => {
        if (!login) {
            navigate("/login")
        }
    }, [])
    
    return(
        <div style={{display:'flex'}}>
            <Container className="sidebar" style={{minHeight: "100vh", maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
                <p className="h2 text-center mt-5" style={{color:'white'}}>Navigation</p>
                <Container className="sidebar-links">
                    <p className="nav-btn text-center active">Home</p>
                    <p className="nav-btn text-center">Aggregate Calculator</p>
                    <p className="nav-btn text-center">Group Former</p>
                    <p className="nav-btn text-center">Quiz Bank</p>
                    <p className="nav-btn text-center">Your Profile</p>
                    <p className="nav-btn text-center" onClick={() => {setToken(null); setLogin(false); navigate("/login")}}><strong>Logout</strong></p>
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
                        <h2 className="text-center">Welcome, You are logged in!</h2>
                        {/* <p className="text-center">Please login to continue</p> */}
                    </Container>
                
                </Container>
            </Container>
        </div>
    )
}