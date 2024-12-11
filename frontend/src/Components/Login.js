import React from "react";
import {Container, Form, Button} from 'react-bootstrap';
import logo from '../Assets/logo.PNG';
export default function Login(){
    return(
        <div style={{display:'flex'}}>
            <Container className="sidebar" style={{minHeight: "100vh", maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
            
            </Container>
            <Container className="main" style={{padding:'0', margin:'0'}}>
                <Container className="header" style={{width:'100%', border:'1px solid black', padding: '0', margin:'0'}}>
                    <Container style={{padding: '20px', textAlign:'center', display:'flex', alignItems:'center', justifyContent:'center'}}>
                        <img src={logo} style={{width:'50px', height:'50px', marginRight:'10px'}} alt="logo" />
                        <p className="h1">StudentSync</p>
                    </Container>
                </Container>
                <Container className="mainContent" style={{backgroundColor:'#D4D4D4', height:'100%'}}>
                    <h2 className="text-center">Login</h2>
                    <p className="text-center">Please login to continue</p>
                    <Form style={{width:'50%', margin:'0 auto'}}>
                        <Form.Group className="mb-3" controlId="formBasicEmail">
                            <Form.Label>Email address</Form.Label>
                            <Form.Control type="email" placeholder="Enter email" />
                            <Form.Label>Password</Form.Label>
                            <Form.Control type="password" placeholder="Enter password" />
                        </Form.Group>
                        <Button variant="primary" type="submit">
                            Login
                        </Button>
                    </Form>
                </Container>
            </Container>
        </div>
    )
}